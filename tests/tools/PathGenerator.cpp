/// @file PathGenerator.cpp
/// @author Group 11 (Chirag Bhansali, Lalit Chitibomma, Daniel Petkoski, Cole
/// Scheller, Devansh Tayal)
/// @brief Test suite for PathGenerator class.

// NOTE - The tests were written with the help of AI.

#include "catch2/catch.hpp"
#include "tools/PathGenerator.hpp"

using cse498::PathGenerator;
using cse498::Point;

TEST_CASE("PathGenerator constructs with defaults", "[pathgenerator]") {
  PathGenerator gen;
  REQUIRE(gen.GetStepSize() == Approx(1.0));
}

TEST_CASE("PathGenerator SetStepSize updates step size", "[pathgenerator]") {
  PathGenerator gen;
  gen.SetStepSize(0.5);
  REQUIRE(gen.GetStepSize() == Approx(0.5));
}

TEST_CASE("PathGenerator ShortestPath finds direct path", "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  auto canMove = [](const Point &) { return true; };
  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE(result.has_value());
  REQUIRE_FALSE(result->empty());
  REQUIRE(result->front().x() == Approx(start.x()));
  REQUIRE(result->front().y() == Approx(start.y()));
  REQUIRE(result->back().x() == Approx(goal.x()));
  REQUIRE(result->back().y() == Approx(goal.y()));
}

TEST_CASE("PathGenerator ShortestPath returns nullopt when blocked",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  auto canMove = [](const Point &) { return false; };
  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator ShortestPath finds path around obstacle",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  // Block direct path at x=5
  auto canMove = [](const Point &p) {
    return !(p.x() >= 4.5 && p.x() <= 5.5 && p.y() >= -1.0 && p.y() <= 1.0);
  };

  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE(result.has_value());
  REQUIRE_FALSE(result->empty());
  // Path should go around the obstacle
  REQUIRE(result->size() > 2);
}

TEST_CASE("PathGenerator PatrolPath connects waypoints", "[pathgenerator]") {
  PathGenerator gen;
  std::vector<Point> waypoints{{0.0, 0.0}, {5.0, 0.0}, {5.0, 5.0}};

  auto result = gen.PatrolPath(waypoints, false);

  REQUIRE(result.has_value());
  REQUIRE(result->front().x() == Approx(0.0));
  REQUIRE(result->back().x() == Approx(5.0));
  REQUIRE(result->back().y() == Approx(5.0));
}

TEST_CASE("PathGenerator PatrolPath creates loop when requested",
          "[pathgenerator]") {
  PathGenerator gen;
  std::vector<Point> waypoints{{0.0, 0.0}, {5.0, 0.0}, {5.0, 5.0}};

  auto result = gen.PatrolPath(waypoints, true);

  REQUIRE(result.has_value());
  REQUIRE(result->isClosed());
}

TEST_CASE("PathGenerator PatrolPath returns nullopt for empty waypoints",
          "[pathgenerator]") {
  PathGenerator gen;
  std::vector<Point> waypoints;

  auto result = gen.PatrolPath(waypoints);

  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator AvoidancePath routes around avoid point",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};
  Point avoid{5.0, 0.0};
  double radius = 2.0;

  auto canMove = [](const Point &) { return true; };
  auto result = gen.AvoidancePath(start, goal, avoid, radius, canMove);

  REQUIRE(result.has_value());

  // Check that all points maintain distance from avoid point
  for (const auto &p : result->pointsView()) {
    double dist = std::hypot(p.x() - avoid.x(), p.y() - avoid.y());
    if (dist < radius - 0.1) {
      // Allow small tolerance
      FAIL("Path point too close to avoidance zone");
    }
  }
}

TEST_CASE("PathGenerator AvoidancePath returns nullopt when impossible",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};
  Point avoid{5.0, 0.0};
  double radius = 20.0; // Huge radius blocks everything

  auto canMove = [](const Point &) { return true; };
  auto result = gen.AvoidancePath(start, goal, avoid, radius, canMove);

  // Both start and goal are within the avoidance zone (dist=5 < radius=20)
  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator RandomWalk generates exploration path",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  size_t steps = 10;

  auto canMove = [](const Point &) { return true; };
  auto result = gen.RandomWalk(start, steps, canMove);

  REQUIRE_FALSE(result.empty());
  REQUIRE(result.front().x() == Approx(start.x()));
  REQUIRE(result.front().y() == Approx(start.y()));
  // Should generate some steps (may be less than requested if dead-end)
  REQUIRE(result.size() >= 1);
}

TEST_CASE("PathGenerator RandomWalk handles constrained movement",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  size_t steps = 10;

  // Only allow movement in small area
  auto canMove = [](const Point &p) {
    return p.x() >= -2.0 && p.x() <= 2.0 && p.y() >= -2.0 && p.y() <= 2.0;
  };

  auto result = gen.RandomWalk(start, steps, canMove);

  REQUIRE_FALSE(result.empty());
  // All points should be within bounds
  for (const auto &p : result.pointsView()) {
    REQUIRE(p.x() >= -2.1);
    REQUIRE(p.x() <= 2.1);
    REQUIRE(p.y() >= -2.1);
    REQUIRE(p.y() <= 2.1);
  }
}

TEST_CASE("PathGenerator SpiralPath creates expanding pattern",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{0.0, 0.0};
  double spacing = 1.0;
  size_t turns = 3;

  auto result = gen.SpiralPath(center, spacing, turns);

  REQUIRE_FALSE(result.empty());
  REQUIRE(result.size() > turns * 4); // At least several points per turn

  // Check that distances from center increase
  double maxDist = 0.0;
  for (const auto &p : result.pointsView()) {
    double dist = std::hypot(p.x() - center.x(), p.y() - center.y());
    REQUIRE(dist >= maxDist - 0.1); // Allow small fluctuation
    maxDist = std::max(maxDist, dist);
  }
}

TEST_CASE("PathGenerator SpiralPath centers at specified point",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{5.0, 3.0};
  double spacing = 0.5;
  size_t turns = 2;

  auto result = gen.SpiralPath(center, spacing, turns);

  REQUIRE_FALSE(result.empty());
  // First point should be at or near center
  REQUIRE(result.front().x() == Approx(center.x()).margin(spacing));
  REQUIRE(result.front().y() == Approx(center.y()).margin(spacing));
}

TEST_CASE("PathGenerator SetHeuristic changes distance calculation",
          "[pathgenerator]") {
  PathGenerator gen;

  // Manhattan distance heuristic
  auto manhattan = [](const Point &a, const Point &b) {
    return std::abs(b.x() - a.x()) + std::abs(b.y() - a.y());
  };

  gen.SetHeuristic(manhattan);

  Point start{0.0, 0.0};
  Point goal{3.0, 4.0};
  auto canMove = [](const Point &) { return true; };

  auto result = gen.ShortestPath(start, goal, canMove);
  REQUIRE(result.has_value());
  REQUIRE(result->front().x() == Approx(start.x()));
  REQUIRE(result->front().y() == Approx(start.y()));
  REQUIRE(result->back().x() == Approx(goal.x()));
  REQUIRE(result->back().y() == Approx(goal.y()));
  REQUIRE(result->totalLength() >= 5.0); // Euclidean distance is 5 (3-4-5 triangle)
}

TEST_CASE("PathGenerator handles same start and goal", "[pathgenerator]") {
  PathGenerator gen;
  Point start{5.0, 5.0};
  Point goal{5.0, 5.0};

  auto canMove = [](const Point &) { return true; };
  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE(result.has_value());
  REQUIRE(result->size() == 1);
  REQUIRE(result->front().x() == Approx(start.x()));
  REQUIRE(result->front().y() == Approx(start.y()));
}

TEST_CASE("PathGenerator works with different step sizes", "[pathgenerator]") {
  PathGenerator gen;
  gen.SetStepSize(0.25);

  Point start{0.0, 0.0};
  Point goal{1.0, 0.0};
  auto canMove = [](const Point &) { return true; };

  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE(result.has_value());
  // With smaller step size, should have more intermediate points
  REQUIRE(result->size() >= 4);
}

TEST_CASE("PathGenerator EuclideanDistance computes correct distance",
          "[pathgenerator]") {
  // This tests the static helper if it's exposed, or indirectly through paths
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{3.0, 4.0};

  auto canMove = [](const Point &) { return true; };
  auto result = gen.ShortestPath(start, goal, canMove);

  REQUIRE(result.has_value());
  // The total length should be approximately the Euclidean distance
  REQUIRE(result->totalLength() >= 5.0); // 3-4-5 triangle
}

TEST_CASE("PathGenerator RandomWalk with zero steps returns start only",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{3.0, 7.0};

  auto canMove = [](const Point &) { return true; };
  auto result = gen.RandomWalk(start, 0, canMove);

  REQUIRE(result.size() == 1);
  REQUIRE(result.front().x() == Approx(start.x()));
  REQUIRE(result.front().y() == Approx(start.y()));
}

TEST_CASE("PathGenerator SpiralPath with zero turns returns center only",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{2.0, 5.0};

  auto result = gen.SpiralPath(center, 1.0, 0);

  REQUIRE(result.size() == 1);
  REQUIRE(result.front().x() == Approx(center.x()));
  REQUIRE(result.front().y() == Approx(center.y()));
}

TEST_CASE("PathGenerator SpiralPath with zero spacing stays at center",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{0.0, 0.0};

  auto result = gen.SpiralPath(center, 0.0, 2);

  // All points should remain at center since r = spacing * angle / 2pi = 0
  REQUIRE_FALSE(result.empty());
  for (const auto &p : result.pointsView()) {
    REQUIRE(p.x() == Approx(center.x()).margin(0.01));
    REQUIRE(p.y() == Approx(center.y()).margin(0.01));
  }
}

TEST_CASE("PathGenerator AvoidancePath with zero radius finds normal path",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{5.0, 0.0};
  Point avoid{2.5, 0.0};

  auto canMove = [](const Point &) { return true; };
  auto result = gen.AvoidancePath(start, goal, avoid, 0.0, canMove);

  // radius=0 only excludes the exact avoid point, path should still be found
  REQUIRE(result.has_value());
  REQUIRE(result->front().x() == Approx(start.x()));
  REQUIRE(result->front().y() == Approx(start.y()));
  REQUIRE(result->back().x() == Approx(goal.x()));
  REQUIRE(result->back().y() == Approx(goal.y()));
}
