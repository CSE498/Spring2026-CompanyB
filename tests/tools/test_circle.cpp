/**
 * @file test_circle.cpp
 * @brief Unit tests for the Circle class (source/tools/Circle.hpp).
 * @author Group-13 (Lemuel). Developed with AI assistance (Cursor).
 *
 * Test files for company C++ classes live in tests/tools/, parallel to source/tools/.
 */

#include <catch2/catch.hpp>
#include <cmath>

#include "Point.hpp"
#include "Circle.hpp"

using namespace cse498;

TEST_CASE("Circle default constructor") {
  Circle c;
  REQUIRE(c.GetCenter() == Point(0.0, 0.0));
  REQUIRE(c.GetRadius() == 0.0);
}

TEST_CASE("Circle contains and boundary checks") {
  Circle c(Point(0.0, 0.0), 2.0);
  REQUIRE(c.Contains(Point(0.0, 0.0)));
  REQUIRE(c.Contains(Point(2.0, 0.0)));
  REQUIRE_FALSE(c.Contains(Point(2.1, 0.0)));

  REQUIRE(c.OnBoundary(Point(2.0, 0.0)));
  REQUIRE_FALSE(c.OnBoundary(Point(1.9, 0.0)));
}

TEST_CASE("Circle overlap and separation") {
  Circle a(Point(0.0, 0.0), 1.0);
  Circle b(Point(2.0, 0.0), 1.0);  // tangent
  Circle c(Point(3.1, 0.0), 1.0);  // separated

  REQUIRE(a.Overlaps(b));
  REQUIRE_FALSE(a.Overlaps(c));

  REQUIRE(a.SeparationDistance(b) == 0.0);
  REQUIRE(std::fabs(c.SeparationDistance(a) - 1.1) < 1e-9);
}

TEST_CASE("Circle translate and scale") {
  Circle c(Point(1.0, 1.0), 2.0);
  c.Translate(Point(2.0, -1.0));
  REQUIRE(c.GetCenter() == Point(3.0, 0.0));

  c.Scale(0.5);
  REQUIRE(std::fabs(c.GetRadius() - 1.0) < 1e-9);
}

TEST_CASE("Circle distance to point and circle") {
  Circle c(Point(0.0, 0.0), 3.0);
  REQUIRE(std::fabs(c.DistanceTo(Point(3.0, 4.0)) - 5.0) < 1e-9);

  Circle other(Point(6.0, 8.0), 1.0);
  REQUIRE(std::fabs(c.DistanceTo(other) - 10.0) < 1e-9);
}

TEST_CASE("Circle overlap when one is inside another") {
  Circle big(Point(0.0, 0.0), 5.0);
  Circle small(Point(1.0, 1.0), 1.0);

  REQUIRE(big.Overlaps(small));
  REQUIRE(small.Overlaps(big));
  REQUIRE(big.SeparationDistance(small) == 0.0);
}

TEST_CASE("Circle boundary epsilon behavior") {
  Circle c(Point(0.0, 0.0), 2.0);
  REQUIRE(c.OnBoundary(Point(2.0 + 1e-10, 0.0)));
  REQUIRE_FALSE(c.OnBoundary(Point(2.0 + 1e-4, 0.0), 1e-6));
}

TEST_CASE("Circle scale by zero") {
  Circle c(Point(2.0, -3.0), 4.0);
  c.Scale(0.0);
  REQUIRE(c.GetRadius() == 0.0);
  REQUIRE(c.Contains(Point(2.0, -3.0)));
}

TEST_CASE("Circle scalar properties") {
  Circle c(Point(0.0, 0.0), 2.0);
  REQUIRE(std::fabs(c.Diameter() - 4.0) < 1e-9);
  REQUIRE(std::fabs(c.Circumference() - (4.0 * Circle::kPi)) < 1e-9);
  REQUIRE(std::fabs(c.Area() - (4.0 * Circle::kPi)) < 1e-9);
}

TEST_CASE("Circle circle-containment and relationship checks") {
  Circle outer(Point(0.0, 0.0), 5.0);
  Circle inner(Point(1.0, 1.0), 2.0);
  Circle far(Point(20.0, 0.0), 1.0);
  Circle tangent(Point(7.0, 0.0), 2.0);

  REQUIRE(outer.Contains(inner));
  REQUIRE_FALSE(inner.Contains(outer));

  REQUIRE(outer.IsDisjoint(far));
  REQUIRE_FALSE(outer.IsDisjoint(inner));

  REQUIRE(outer.IsTangentTo(tangent));
}

TEST_CASE("Circle signed distance to boundary") {
  Circle c(Point(0.0, 0.0), 3.0);
  REQUIRE(std::fabs(c.SignedDistanceToBoundary(Point(3.0, 0.0))) < 1e-9);
  REQUIRE(c.SignedDistanceToBoundary(Point(0.0, 0.0)) < 0.0);
  REQUIRE(c.SignedDistanceToBoundary(Point(5.0, 0.0)) > 0.0);
}

TEST_CASE("Circle intersection points two-point case") {
  Circle a(Point(0.0, 0.0), 5.0);
  Circle b(Point(8.0, 0.0), 5.0);

  const auto pts = a.IntersectionPoints(b);
  REQUIRE(pts.size() == 2);

  REQUIRE(a.OnBoundary(pts[0]));
  REQUIRE(a.OnBoundary(pts[1]));
  REQUIRE(b.OnBoundary(pts[0]));
  REQUIRE(b.OnBoundary(pts[1]));
}

TEST_CASE("Circle intersection points tangent and disjoint") {
  Circle a(Point(0.0, 0.0), 3.0);
  Circle tangent(Point(6.0, 0.0), 3.0);
  Circle disjoint(Point(10.0, 0.0), 3.0);

  const auto tangent_pts = a.IntersectionPoints(tangent);
  REQUIRE(tangent_pts.size() == 1);
  REQUIRE(a.OnBoundary(tangent_pts[0]));
  REQUIRE(tangent.OnBoundary(tangent_pts[0]));

  const auto disjoint_pts = a.IntersectionPoints(disjoint);
  REQUIRE(disjoint_pts.empty());
}
