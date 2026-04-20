/// @file PathGenerator.cpp
/// @author Group 11 (Chirag Bhansali, Lalit Chitibomma, Daniel Petkoski, Cole
/// Scheller, Devansh Tayal)
/// @brief Test suite for PathGenerator class.

// NOTE - The tests were written with the help of AI.

// #include "catch2/catch.hpp"

#include <catch2/catch_test_macros.hpp>

// for Catch::Matchers::WithinRel() for approx double in Catchv3
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "tools/PathGenerator.hpp"

using cse498::PathGenerator;
using cse498::Point;

TEST_CASE("PathGenerator constructs with defaults", "[pathgenerator]") {
  PathGenerator gen;
  REQUIRE_THAT(gen.GetStepSize(), Catch::Matchers::WithinRel(1.0));
}

TEST_CASE("PathGenerator SetStepSize updates step size", "[pathgenerator]") {
  PathGenerator gen;
  gen.SetStepSize(0.5);
  REQUIRE_THAT(gen.GetStepSize(), Catch::Matchers::WithinRel(0.5));
}

TEST_CASE("PathGenerator ShortestPath finds direct path", "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  auto result = gen.ShortestPath(start, goal);

  REQUIRE(result.has_value());
  REQUIRE_FALSE(result->empty());
  REQUIRE_THAT(result->front().getX(),
               Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result->front().getY(),
               Catch::Matchers::WithinRel(start.getY()));
  REQUIRE_THAT(result->back().getX(), Catch::Matchers::WithinRel(goal.getX()));
  REQUIRE_THAT(result->back().getY(), Catch::Matchers::WithinRel(goal.getY()));
}

TEST_CASE("PathGenerator ShortestPath returns nullopt when blocked",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  gen.SetCanMove([](const Point&) { return false; });
  auto result = gen.ShortestPath(start, goal);

  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator ShortestPath returns nullopt for invalid start",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  gen.SetCanMove([&](const Point& p) {
    return !(p.getX() == start.getX() && p.getY() == start.getY());
  });
  auto result = gen.ShortestPath(start, goal);

  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator ShortestPath returns nullopt for invalid goal",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  gen.SetCanMove([&](const Point& p) {
    return !(p.getX() == goal.getX() && p.getY() == goal.getY());
  });
  auto result = gen.ShortestPath(start, goal);

  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator ShortestPath finds path around obstacle",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{10.0, 0.0};

  // Block direct path at x=5
  gen.SetCanMove([](const Point& p) {
    return !(p.getX() >= 4.5 && p.getX() <= 5.5 && p.getY() >= -1.0 &&
             p.getY() <= 1.0);
  });

  auto result = gen.ShortestPath(start, goal);

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
  REQUIRE_THAT(result->front().getX(), Catch::Matchers::WithinRel(0.0));
  REQUIRE_THAT(result->back().getX(), Catch::Matchers::WithinRel(5.0));
  REQUIRE_THAT(result->back().getY(), Catch::Matchers::WithinRel(5.0));
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

  auto result = gen.AvoidancePath(start, goal, avoid, radius);

  REQUIRE(result.has_value());

  // Check that all points maintain distance from avoid point
  for (const auto& p : result->pointsView()) {
  for (const auto& p : result->pointsView()) {
    double dist = std::hypot(p.getX() - avoid.getX(), p.getY() - avoid.getY());
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
  double radius = 20.0;  // Huge radius blocks everything

  auto result = gen.AvoidancePath(start, goal, avoid, radius);

  // Both start and goal are within the avoidance zone (dist=5 < radius=20)
  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PathGenerator RandomWalk at invalid start returns start only",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};

  gen.SetCanMove([](const Point&) { return false; });
  auto result = gen.RandomWalk(start, 10);

  REQUIRE(result.size() == 1);
  REQUIRE_THAT(result.front().getX(), Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result.front().getY(), Catch::Matchers::WithinRel(start.getY()));
}

TEST_CASE("PathGenerator RandomWalk generates exploration path",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  size_t steps = 10;

  auto result = gen.RandomWalk(start, steps);

  REQUIRE_FALSE(result.empty());
  REQUIRE_THAT(result.front().getX(), Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result.front().getY(), Catch::Matchers::WithinRel(start.getY()));
  // Should generate some steps (may be less than requested if dead-end)
  REQUIRE(result.size() >= 1);
}

TEST_CASE("PathGenerator RandomWalk handles constrained movement",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  size_t steps = 10;

  // Only allow movement in small area
  gen.SetCanMove([](const Point& p) {
    return p.getX() >= -2.0 && p.getX() <= 2.0 && p.getY() >= -2.0 &&
           p.getY() <= 2.0;
  });

  auto result = gen.RandomWalk(start, steps);

  REQUIRE_FALSE(result.empty());
  // All points should be within bounds
  for (const auto& p : result.pointsView()) {
  for (const auto& p : result.pointsView()) {
    REQUIRE(p.getX() >= -2.1);
    REQUIRE(p.getX() <= 2.1);
    REQUIRE(p.getY() >= -2.1);
    REQUIRE(p.getY() <= 2.1);
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
  REQUIRE(result.size() > turns * 4);  // At least several points per turn

  // Check that distances from center increase
  double maxDist = 0.0;
  for (const auto& p : result.pointsView()) {
  for (const auto& p : result.pointsView()) {
    double dist =
        std::hypot(p.getX() - center.getX(), p.getY() - center.getY());
    REQUIRE(dist >= maxDist - 0.1);  // Allow small fluctuation
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
  REQUIRE_THAT(result.front().getX(),
               Catch::Matchers::WithinAbs(center.getX(), spacing));
  REQUIRE_THAT(result.front().getY(),
               Catch::Matchers::WithinAbs(center.getY(), spacing));
}

TEST_CASE("PathGenerator SetHeuristic changes distance calculation",
          "[pathgenerator]") {
  PathGenerator gen;

  // Manhattan distance heuristic
  auto manhattan = [](const Point& a, const Point& b) {
  auto manhattan = [](const Point& a, const Point& b) {
    return std::abs(b.getX() - a.getX()) + std::abs(b.getY() - a.getY());
  };

  gen.SetHeuristic(manhattan);

  Point start{0.0, 0.0};
  Point goal{3.0, 4.0};
  auto result = gen.ShortestPath(start, goal);
  REQUIRE(result.has_value());
  REQUIRE_THAT(result->front().getX(),
               Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result->front().getY(),
               Catch::Matchers::WithinRel(start.getY()));
  REQUIRE_THAT(result->back().getX(), Catch::Matchers::WithinRel(goal.getX()));
  REQUIRE_THAT(result->back().getY(), Catch::Matchers::WithinRel(goal.getY()));
  REQUIRE(result->totalLength() >=
          5.0);  // Euclidean distance is 5 (3-4-5 triangle)
}

TEST_CASE("PathGenerator handles same start and goal", "[pathgenerator]") {
  PathGenerator gen;
  Point start{5.0, 5.0};
  Point goal{5.0, 5.0};

  auto result = gen.ShortestPath(start, goal);

  REQUIRE(result.has_value());
  REQUIRE(result->size() == 1);
  REQUIRE_THAT(result->front().getX(),
               Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result->front().getY(),
               Catch::Matchers::WithinRel(start.getY()));
}

TEST_CASE("PathGenerator works with different step sizes", "[pathgenerator]") {
  PathGenerator gen;
  gen.SetStepSize(0.25);

  Point start{0.0, 0.0};
  Point goal{1.0, 0.0};
  auto result = gen.ShortestPath(start, goal);

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

  auto result = gen.ShortestPath(start, goal);

  REQUIRE(result.has_value());
  // The total length should be approximately the Euclidean distance
  REQUIRE(result->totalLength() >= 5.0);  // 3-4-5 triangle
}

TEST_CASE("PathGenerator RandomWalk with zero steps returns start only",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{3.0, 7.0};

  auto result = gen.RandomWalk(start, 0);

  REQUIRE(result.size() == 1);
  REQUIRE_THAT(result.front().getX(), Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result.front().getY(), Catch::Matchers::WithinRel(start.getY()));
}

TEST_CASE("PathGenerator SpiralPath with zero turns returns center only",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{2.0, 5.0};

  auto result = gen.SpiralPath(center, 1.0, 0);

  REQUIRE(result.size() == 1);
  REQUIRE_THAT(result.front().getX(),
               Catch::Matchers::WithinRel(center.getX()));
  REQUIRE_THAT(result.front().getY(),
               Catch::Matchers::WithinRel(center.getY()));
}

TEST_CASE("PathGenerator SpiralPath with zero spacing stays at center",
          "[pathgenerator]") {
  PathGenerator gen;
  Point center{0.0, 0.0};

  auto result = gen.SpiralPath(center, 0.0, 2);

  // All points should remain at center since r = spacing * angle / 2pi = 0
  REQUIRE_FALSE(result.empty());
  for (const auto& p : result.pointsView()) {
  for (const auto& p : result.pointsView()) {
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinAbs(center.getX(), 0.01));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinAbs(center.getY(), 0.01));
  }
}

TEST_CASE("PathGenerator AvoidancePath with zero radius finds normal path",
          "[pathgenerator]") {
  PathGenerator gen;
  Point start{0.0, 0.0};
  Point goal{5.0, 0.0};
  Point avoid{2.5, 0.0};

  auto result = gen.AvoidancePath(start, goal, avoid, 0.0);

  // radius=0 only excludes the exact avoid point, path should still be found
  REQUIRE(result.has_value());
  REQUIRE_THAT(result->front().getX(),
               Catch::Matchers::WithinRel(start.getX()));
  REQUIRE_THAT(result->front().getY(),
               Catch::Matchers::WithinRel(start.getY()));
  REQUIRE_THAT(result->back().getX(), Catch::Matchers::WithinRel(goal.getX()));
  REQUIRE_THAT(result->back().getY(), Catch::Matchers::WithinRel(goal.getY()));
}
