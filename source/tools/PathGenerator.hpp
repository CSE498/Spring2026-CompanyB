/// @file PathGenerator.hpp
/// @author Group 11 (Chirag Bhansali, Lalit Chitibomma, Daniel Petkoski, Cole
/// Scheller, Devansh Tayal)
/// @brief Pathfinding and path generation utilities for agent navigation.

// NOTE - Some methods and comments were written with the help of AI

#pragma once

#include <array>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Point.hpp"
#include "WorldPath.hpp"

namespace cse498 {

/// Type alias for world query callback functions
using WorldQueryFunc = std::function<bool(const Point&)>;

/// Type alias for distance heuristic callback functions
using HeuristicFunc = std::function<double(const Point&, const Point&)>;

struct PointHash {
  std::size_t operator()(const Point& p) const noexcept {
    return std::hash<double>{}(p.getX()) ^ (std::hash<double>{}(p.getY()) << 1);
  }
};

using PointMap = std::unordered_map<Point, Point, PointHash>;

/**
 * @brief Pathfinding and path generation for agent navigation.
 *
 * Provides A* pathfinding, patrol routes, avoidance paths, random walks,
 * and procedural path generation (spirals). Configurable with custom
 * heuristics and step sizes for different world representations.
 */
class PathGenerator {
 public:
  // Two points closer than (step_size_ * kCoincidentEps) are treated as the
  // same location. E.g., with step_size_=1.0, points within 0.01 units apart
  // are considered coincident.
  static constexpr double kCoincidentEps = 0.01;
  // A node is considered to have reached the goal when its distance to the
  // goal is less than (step_size_ * kGoalEps). E.g., with step_size_=1.0, a
  // node within 0.6 units of the goal is accepted as "arrived".
  static constexpr double kGoalEps = 0.6;
  // Maximum A* iterations before giving up, to prevent infinite loops in
  // pathological cases (e.g., very large or disconnected worlds).
  static constexpr std::size_t kMaxIterations = 10000;

  PathGenerator() = default;

  // ========== Core Path Generation ==========

  /**
   * @brief Generate shortest path from start to goal using A* algorithm.
   * @param start Starting point
   * @param goal Goal point
   * @return Optional WorldPath (nullopt if no path exists)
   */
  [[nodiscard]] std::optional<WorldPath> ShortestPath(const Point& start,
                                                      const Point& goal) const;

  /**
   * @brief Generate a patrol path through waypoints.
   * @param waypoints Points to visit in order
   * @param loop If true, return to start point
   * @return Optional WorldPath (nullopt if waypoints are unreachable)
   */
  [[nodiscard]] std::optional<WorldPath> PatrolPath(
      const std::vector<Point>& waypoints, bool loop = true) const;

  /**
   * @brief Generate path from start to goal while avoiding a region.
   * @param start Starting point
   * @param goal Goal point
   * @param avoid Center of region to avoid
   * @param radius Radius around avoid point to stay away from
   * @return Optional WorldPath (nullopt if avoidance makes path impossible)
   */
  [[nodiscard]] std::optional<WorldPath> AvoidancePath(const Point& start,
                                                       const Point& goal,
                                                       const Point& avoid,
                                                       double radius) const;

  // ========== Utility Generation ==========

  /**
   * @brief Generate a random walk path for exploration.
   * @param start Starting point
   * @param steps Number of steps to take
   * @return WorldPath (returns partial path if dead-end reached)
   */
  [[nodiscard]] WorldPath RandomWalk(const Point& start, size_t steps) const;

  /**
   * @brief Generate an expanding spiral path centered at a point.
   * @param center Center of the spiral
   * @param spacing Distance between spiral turns
   * @param turns Number of complete rotations
   * @return WorldPath representing the spiral
   */
  [[nodiscard]] WorldPath SpiralPath(const Point& center, double spacing,
                                     size_t turns) const;

  // ========== Configuration ==========

  /**
   * @brief Set the traversability predicate used by all path generation methods.
   * @param f Returns true if a point is passable. Defaults to always-true.
   */
  void SetCanMove(WorldQueryFunc f);

  /**
   * @brief Set the distance heuristic for pathfinding algorithms.
   * @param h Heuristic function (e.g., Manhattan, Euclidean, Chebyshev)
   */
  void SetHeuristic(HeuristicFunc h);

  /**
   * @brief Set step size for continuous world pathfinding.
   * @param size Step size (must be positive)
   */
  void SetStepSize(double size);

  /**
   * @brief Get current step size.
   * @return Current step size value
   */
  [[nodiscard]] double GetStepSize() const { return step_size_; }

 private:
  /// Traversability predicate; returns true if a point can be visited.
  /// Defaults to always-true (open world with no obstacles).
  WorldQueryFunc canMove_{[](const Point&) { return true; }};
  HeuristicFunc heuristic_{EuclideanDistance};  ///< Distance estimation function
  double step_size_{1.0};                       ///< Granularity for continuous worlds

  // ========== Helper Functions ==========

  /**
   * @brief Default Euclidean distance heuristic.
   */
  static double EuclideanDistance(const Point& a, const Point& b);

  /**
   * @brief Get neighboring points for pathfinding expansion.
   * @param p Current point
   * @return Vector of neighboring points based on step_size_
   */
  [[nodiscard]] std::vector<Point> GetNeighbors(const Point& p) const;

  /**
   * @brief Reconstruct path from A* search results.
   * @param came_from Map of point -> previous point
   * @param current Final point reached
   * @return Reconstructed WorldPath
   */
  [[nodiscard]] WorldPath ReconstructPath(const PointMap& came_from,
                                          const Point& current) const;

  /// Core A* implementation used by ShortestPath and AvoidancePath.
  [[nodiscard]] std::optional<WorldPath> ShortestPathImpl(
      const Point& start, const Point& goal,
      const WorldQueryFunc& canMove) const;

  /// Unit direction vectors for 8-connected grid expansion (cardinal +
  /// diagonal). Stored here so any method can reference kUnitDirections.size()
  /// without repeating the literal 8.
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
};

}  // namespace cse498
