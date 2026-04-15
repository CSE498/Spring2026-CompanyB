/// @file PathGenerator.cpp
/// @author Group 11 (Chirag Bhansali, Lalit Chitibomma, Daniel Petkoski, Cole
/// Scheller, Devansh Tayal)
/// @brief Pathfinding and path generation utilities for agent navigation.

// NOTE - Some methods and comments were written with the help of AI

#include "tools/PathGenerator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <queue>
#include <random>
#include <unordered_set>

namespace cse498 {

PathGenerator::PathGenerator()
    : heuristic_(EuclideanDistance), step_size_(1.0) {}

// ========== Core Path Generation ==========

std::optional<WorldPath> PathGenerator::ShortestPath(
    const Point& start, const Point& goal,
    const WorldQueryFunc& canMove) const {
  assert(canMove && "WorldQueryFunc cannot be null");

  // Handle degenerate case: start == goal
  if (std::abs(start.getX() - goal.getX()) <
          step_size_ * kPointCoincidentFraction &&
      std::abs(start.getY() - goal.getY()) <
          step_size_ * kPointCoincidentFraction) {
    WorldPath path;
    path.addPoint(start);
    return path;
  }

  // Check if start and goal are valid
  if (!canMove(start) || !canMove(goal)) {
    return std::nullopt;
  }

  // A* algorithm implementation
  using PointDist = std::pair<double, Point>;
  using PointSet = std::unordered_set<Point, PointHash>;
  using PointScoreMap = std::unordered_map<Point, double, PointHash>;
  auto cmp = [](const PointDist& a, const PointDist& b) {
    return a.first > b.first;  // Min-heap based on f-score
  };
  std::priority_queue<PointDist, std::vector<PointDist>, decltype(cmp)> openSet(
      cmp);

  PointSet closedSet;  // Track visited nodes
  PointMap cameFrom;
  PointScoreMap gScore;

  gScore[start] = 0.0;
  double fStart = heuristic_(start, goal);
  openSet.push({fStart, start});

  size_t iterations = 0;
  const size_t maxIterations = 10000;  // Prevent infinite loops

  while (!openSet.empty() && iterations++ < maxIterations) {
    Point current = openSet.top().second;
    openSet.pop();

    // Skip if already visited
    if (closedSet.count(current) > 0) {
      continue;
    }

    closedSet.insert(current);

    // Check if we reached the goal
    double distToGoal = heuristic_(current, goal);
    if (distToGoal < step_size_ * kGoalReachedFraction) {
      // Close enough to goal - add goal point and return
      if (distToGoal > step_size_ * kPointCoincidentFraction) {
        cameFrom[goal] = current;
        return ReconstructPath(cameFrom, goal);
      } else {
        return ReconstructPath(cameFrom, current);
      }
    }

    // Explore neighbors
    std::vector<Point> neighbors = GetNeighbors(current);
    for (const Point& neighbor : neighbors) {
      if (closedSet.count(neighbor) > 0 || !canMove(neighbor)) continue;

      double tentativeGScore = gScore[current] + heuristic_(current, neighbor);

      if (gScore.find(neighbor) == gScore.end() ||
          tentativeGScore < gScore[neighbor]) {
        cameFrom[neighbor] = current;
        gScore[neighbor] = tentativeGScore;
        double fScore = gScore[neighbor] + heuristic_(neighbor, goal);
        openSet.push({fScore, neighbor});
      }
    }
  }

  // No path found
  return std::nullopt;
}

std::optional<WorldPath> PathGenerator::PatrolPath(
    const std::vector<Point>& waypoints, bool loop) const {
  if (waypoints.empty()) {
    return std::nullopt;
  }

  if (waypoints.size() == 1) {
    WorldPath path;
    path.addPoint(waypoints[0]);
    return path;
  }

  WorldPath finalPath;
  auto canMoveAnywhere = [](const Point&) { return true; };

  // Connect each waypoint to the next
  for (size_t i = 0; i < waypoints.size() - 1; ++i) {
    auto segment =
        ShortestPath(waypoints[i], waypoints[i + 1], canMoveAnywhere);
    if (!segment.has_value()) {
      // Skip unreachable waypoints (soft error per docs)
      continue;
    }

    // Add points, avoiding duplicates at waypoint junctions
    for (size_t j = 0; j < segment->size(); ++j) {
      if (i > 0 && j == 0 && !finalPath.empty()) {
        // Skip first point of segment if it duplicates last point
        continue;
      }
      finalPath.addPoint((*segment)[j]);
    }
  }

  // If loop requested, connect back to start
  if (loop && !waypoints.empty() && !finalPath.empty()) {
    auto lastSegment =
        ShortestPath(waypoints.back(), waypoints.front(), canMoveAnywhere);
    if (lastSegment.has_value()) {
      for (size_t j = 1; j < lastSegment->size(); ++j) {
        finalPath.addPoint((*lastSegment)[j]);
      }
    }
  }

  return finalPath.empty() ? std::nullopt : std::optional<WorldPath>(finalPath);
}

std::optional<WorldPath> PathGenerator::AvoidancePath(
    const Point& start, const Point& goal, const Point& avoid, double radius,
    const WorldQueryFunc& canMove) const {
  assert(canMove && "WorldQueryFunc cannot be null");
  assert(radius >= 0.0 && "Avoidance radius cannot be negative");

  // Create a combined movement validator
  auto canMoveWithAvoidance = [&](const Point& p) {
    if (!canMove(p)) return false;
    double dist = std::hypot(p.getX() - avoid.getX(), p.getY() - avoid.getY());
    return dist >= radius;
  };

  return ShortestPath(start, goal, canMoveWithAvoidance);
}

// ========== Utility Generation ==========

WorldPath PathGenerator::RandomWalk(const Point& start, size_t steps,
                                    const WorldQueryFunc& canMove) const {
  assert(canMove && "WorldQueryFunc cannot be null");

  WorldPath path;
  path.addPoint(start);

  if (steps == 0 || !canMove(start)) {
    return path;
  }

  static std::random_device rd;
  static std::mt19937 gen(rd());

  Point current = start;
  std::vector<Point> neighbors;
  std::vector<Point> validNeighbors;
  validNeighbors.reserve(8);

  for (size_t i = 0; i < steps; ++i) {
    validNeighbors.clear();
    neighbors = GetNeighbors(current);

    for (const Point& neighbor : neighbors) {
      if (canMove(neighbor)) {
        validNeighbors.push_back(neighbor);
      }
    }

    if (validNeighbors.empty()) {
      // Dead end - return partial path
      break;
    }

    // Pick random valid neighbor
    std::uniform_int_distribution<> neighborDis(0, validNeighbors.size() - 1);
    current = validNeighbors[neighborDis(gen)];
    path.addPoint(current);
  }

  return path;
}

WorldPath PathGenerator::SpiralPath(const Point& center, double spacing,
                                    size_t turns) const {
  WorldPath path;

  if (turns == 0) {
    path.addPoint(center);
    return path;
  }

  // Start at center
  path.addPoint(center);

  // Generate spiral using parametric equations
  // r(t) = spacing * t / (2*pi)
  // x(t) = center.getX() + r(t) * cos(t)
  // y(t) = center.getY() + r(t) * sin(t)

  const double maxAngle = turns * 2.0 * M_PI;
  const double angleStep = step_size_ / (spacing > 0 ? spacing : 1.0);

  for (double angle = angleStep; angle <= maxAngle; angle += angleStep) {
    double r = (spacing * angle) / (2.0 * M_PI);
    Point p(center.getX() + r * std::cos(angle),
            center.getY() + r * std::sin(angle));
    path.addPoint(p);
  }

  return path;
}

// ========== Configuration ==========

void PathGenerator::SetHeuristic(HeuristicFunc h) {
  assert(h && "HeuristicFunc cannot be null");
  heuristic_ = h;
}

void PathGenerator::SetStepSize(double size) {
  assert(size > 0.0 && "Step size must be positive");
  step_size_ = size;
}

// ========== Helper Functions ==========

double PathGenerator::EuclideanDistance(const Point& a, const Point& b) {
  double dx = b.getX() - a.getX();
  double dy = b.getY() - a.getY();
  return std::hypot(dx, dy);
}

std::vector<Point> PathGenerator::GetNeighbors(const Point& p) const {
  static constexpr std::array<std::pair<double, double>, 8> kUnitDirections{{
      {1.0, 0.0},
      {-1.0, 0.0},
      {0.0, 1.0},
      {0.0, -1.0},
      {1.0, 1.0},
      {1.0, -1.0},
      {-1.0, 1.0},
      {-1.0, -1.0},
  }};

  std::vector<Point> neighbors;
  neighbors.reserve(kUnitDirections.size());

  const double step = step_size_;
  for (const auto& [ux, uy] : kUnitDirections) {
    neighbors.emplace_back(p.getX() + ux * step, p.getY() + uy * step);
  }

  return neighbors;
}

WorldPath PathGenerator::ReconstructPath(const PointMap& came_from,
                                         const Point& current) const {
  std::vector<Point> points;
  for (Point node = current;;) {
    points.push_back(node);
    auto it = came_from.find(node);
    if (it == came_from.end()) break;
    node = it->second;
  }
  std::reverse(points.begin(), points.end());
  WorldPath path;
  for (const auto& p : points) path.addPoint(p);
  return path;
}

}  // namespace cse498
