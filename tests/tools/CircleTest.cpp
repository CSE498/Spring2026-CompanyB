/**
 * @file CircleTest.cpp
 * @brief Unit tests for the Circle class (source/tools/Circle.hpp).
 * @author Group-13 (Lemuel). Developed with AI assistance.
 *
 * Test files for company C++ classes live in tests/tools/, parallel to source/tools/.
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <cmath>
#include <stdexcept>

#include "../../source/tools/PointClass.hpp"
#include "../../source/tools/Circle.hpp"

using namespace cse498;

TEST_CASE("Circle default constructor") {
  Circle c;
  REQUIRE(c.GetCenter() == Point(0.0, 0.0));
  REQUIRE(c.GetRadius() == 0.0);
}

TEST_CASE("Circle setters update center and radius") {
  Circle c;
  c.SetCenter(Point(4.0, -2.0));
  c.SetRadius(3.5);
  REQUIRE(c.GetCenter() == Point(4.0, -2.0));
  REQUIRE(std::fabs(c.GetRadius() - 3.5) < 1e-9);
}

TEST_CASE("Circle rejects invalid radius and non-positive scale") {
  REQUIRE_THROWS_AS(Circle(Point(0.0, 0.0), -1.0), std::invalid_argument);

  Circle c(Point(0.0, 0.0), 2.0);
  REQUIRE_THROWS_AS(c.SetRadius(-0.1), std::invalid_argument);
  REQUIRE_THROWS_AS(c.Scale(0.0), std::invalid_argument);
  REQUIRE_THROWS_AS(c.Scale(-2.0), std::invalid_argument);
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
  REQUIRE(std::fabs(a.SeparationDistance(c) - c.SeparationDistance(a)) < 1e-9);
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
  REQUIRE(std::fabs(c.CenterDistanceTo(other) - 10.0) < 1e-9);

  REQUIRE(std::fabs(c.DistanceTo(other) - 10.0) < 1e-9);  // backward-compatible alias
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

TEST_CASE("Circle scale by zero throws and leaves state unchanged") {
  Circle c(Point(2.0, -3.0), 4.0);
  REQUIRE_THROWS_AS(c.Scale(0.0), std::invalid_argument);
  REQUIRE(std::fabs(c.GetRadius() - 4.0) < 1e-9);
  REQUIRE(c.GetCenter() == Point(2.0, -3.0));
}

TEST_CASE("Circle scalar properties") {
  Circle c(Point(0.0, 0.0), 2.0);
  REQUIRE(std::fabs(c.Diameter() - 4.0) < 1e-9);
  REQUIRE(std::fabs(c.Circumference() - (4.0 * Circle::PI)) < 1e-9);
  REQUIRE(std::fabs(c.Area() - (4.0 * Circle::PI)) < 1e-9);

  Circle degenerate(Point(1.0, 1.0), 0.0);
  REQUIRE(degenerate.Diameter() == 0.0);
  REQUIRE(degenerate.Circumference() == 0.0);
  REQUIRE(degenerate.Area() == 0.0);
}

TEST_CASE("Circle circle-containment and relationship checks") {
  Circle outer(Point(0.0, 0.0), 5.0);
  Circle inner(Point(1.0, 1.0), 2.0);
  Circle inner_tangent(Point(3.0, 0.0), 2.0);
  Circle far(Point(20.0, 0.0), 1.0);
  Circle tangent(Point(7.0, 0.0), 2.0);

  REQUIRE(outer.Contains(inner));
  REQUIRE(outer.Contains(inner_tangent));
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

TEST_CASE("Circle try-intersection optional API") {
  Circle a(Point(0.0, 0.0), 5.0);
  Circle b(Point(8.0, 0.0), 5.0);
  Circle disjoint(Point(30.0, 0.0), 2.0);
  Circle coincident(Point(0.0, 0.0), 5.0);

  const auto two_point_intersection = a.TryIntersectionPoints(b);
  REQUIRE(two_point_intersection.has_value());
  REQUIRE(two_point_intersection->size() == 2);

  const auto no_intersection = a.TryIntersectionPoints(disjoint);
  REQUIRE_FALSE(no_intersection.has_value());

  const auto coincident_intersection = a.TryIntersectionPoints(coincident);
  REQUIRE_FALSE(coincident_intersection.has_value());
}

TEST_CASE("Circle intersection points concentric circles return empty") {
  Circle a(Point(0.0, 0.0), 3.0);
  Circle same(Point(0.0, 0.0), 3.0);
  Circle different_radius(Point(0.0, 0.0), 5.0);

  REQUIRE(a.IntersectionPoints(same).empty());
  REQUIRE(a.IntersectionPoints(different_radius).empty());
}

TEST_CASE("Circle equality operator") {
  Circle a(Point(1.0, 2.0), 3.0);
  Circle b(Point(1.0, 2.0), 3.0);
  Circle c(Point(1.0, 2.0), 4.0);
  Circle nearly_equal(Point(1.0 + 0.5 * Circle::EPS, 2.0), 3.0 + 0.5 * Circle::EPS);
  Circle beyond_tolerance(Point(1.0 + 2.0 * Circle::EPS, 2.0), 3.0);

  REQUIRE(a == b);
  REQUIRE(a == nearly_equal);
  REQUIRE(a != beyond_tolerance);
  REQUIRE(a != c);
}
