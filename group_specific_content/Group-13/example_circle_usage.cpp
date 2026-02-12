/**
 * @file example_circle_usage.cpp
 * @brief Example usage of the Circle class (Group-13).
 *
 * Demonstrates construction, containment, overlap, distances, scalar properties,
 * intersection points, and transformations.
 *
 * Compile with: g++ -std=c++17 example_circle_usage.cpp -o example_circle
 * (from group_specific_content/Group-13 with Point.h and Circle.h in the same directory)
 */

#include "Circle.h"
#include "Point.h"
#include <iostream>
#include <iomanip>
#include <cmath>

static void DemonstrateConstruction() {
  std::cout << "=== Construction ===\n\n";

  Circle c1;
  std::cout << "Default: " << c1 << "\n";
  std::cout << "  Center: (" << c1.GetCenter().x() << ", " << c1.GetCenter().y()
            << "), radius = " << c1.GetRadius() << "\n\n";

  Circle c2(Point(1.0, -2.0), 3.0);
  std::cout << "From center + radius: " << c2 << "\n";
  std::cout << "  Diameter = " << c2.Diameter()
            << ", Circumference = " << c2.Circumference()
            << ", Area = " << c2.Area() << "\n\n";
}

static void DemonstrateContainment() {
  std::cout << "=== Containment ===\n\n";

  Circle c(Point(0.0, 0.0), 2.0);
  std::cout << "Circle " << c << "\n\n";

  auto test = [&c](double x, double y) {
    Point p(x, y);
    std::cout << "  Point (" << x << ", " << y << "): "
              << (c.Contains(p) ? "inside" : "outside")
              << ", on boundary: " << (c.OnBoundary(p) ? "yes" : "no")
              << ", signed dist to boundary: " << c.SignedDistanceToBoundary(p) << "\n";
  };

  test(0, 0);
  test(2, 0);
  test(2.1, 0);
  test(1, 1);
  std::cout << "\n";
}

static void DemonstrateOverlapAndSeparation() {
  std::cout << "=== Overlap & Separation ===\n\n";

  Circle a(Point(0.0, 0.0), 1.0);
  Circle b(Point(2.0, 0.0), 1.0);   // tangent
  Circle d(Point(3.5, 0.0), 1.0);   // disjoint

  std::cout << "Circle A: " << a << "\n";
  std::cout << "Circle B (tangent): " << b << "\n";
  std::cout << "Circle D (disjoint): " << d << "\n\n";

  std::cout << "  A overlaps B: " << (a.Overlaps(b) ? "yes" : "no") << "\n";
  std::cout << "  A overlaps D: " << (a.Overlaps(d) ? "yes" : "no") << "\n";
  std::cout << "  A is disjoint from D: " << (a.IsDisjoint(d) ? "yes" : "no") << "\n";
  std::cout << "  A is tangent to B: " << (a.IsTangentTo(b) ? "yes" : "no") << "\n";
  std::cout << "  A separation from B: " << a.SeparationDistance(b) << "\n";
  std::cout << "  A separation from D: " << a.SeparationDistance(d) << "\n\n";
}

static void DemonstrateDistances() {
  std::cout << "=== Distances ===\n\n";

  Circle c(Point(0.0, 0.0), 3.0);
  Point p(3.0, 4.0);
  std::cout << "Circle " << c << ", point (" << p.x() << ", " << p.y() << ")\n";
  std::cout << "  Distance from center to point: " << c.DistanceTo(p) << " (expect 5)\n\n";

  Circle other(Point(6.0, 8.0), 1.0);
  std::cout << "Other circle center (6, 8), radius 1\n";
  std::cout << "  Center-to-center distance: " << c.DistanceTo(other) << " (expect 10)\n\n";
}

static void DemonstrateCircleContainment() {
  std::cout << "=== Circle-in-Circle ===\n\n";

  Circle outer(Point(0.0, 0.0), 5.0);
  Circle inner(Point(1.0, 1.0), 2.0);
  Circle far(Point(20.0, 0.0), 1.0);

  std::cout << "Outer: " << outer << "\n";
  std::cout << "Inner: " << inner << "\n";
  std::cout << "Far:   " << far << "\n\n";
  std::cout << "  Outer contains Inner: " << (outer.Contains(inner) ? "yes" : "no") << "\n";
  std::cout << "  Inner contains Outer: " << (inner.Contains(outer) ? "yes" : "no") << "\n";
  std::cout << "  Outer contains Far:   " << (outer.Contains(far) ? "yes" : "no") << "\n\n";
}

static void DemonstrateIntersections() {
  std::cout << "=== Intersection Points ===\n\n";

  Circle a(Point(0.0, 0.0), 5.0);
  Circle b(Point(8.0, 0.0), 5.0);
  auto pts = a.IntersectionPoints(b);
  std::cout << "Two circles (centers 8 apart, radii 5): " << pts.size() << " intersection(s)\n";
  for (size_t i = 0; i < pts.size(); ++i)
    std::cout << "  P" << (i + 1) << " = (" << pts[i].x() << ", " << pts[i].y() << ")\n";
  std::cout << "\n";

  Circle t1(Point(0.0, 0.0), 3.0);
  Circle t2(Point(6.0, 0.0), 3.0);
  auto tangent_pts = t1.IntersectionPoints(t2);
  std::cout << "Tangent circles: " << tangent_pts.size() << " intersection(s)\n";
  for (size_t i = 0; i < tangent_pts.size(); ++i)
    std::cout << "  (" << tangent_pts[i].x() << ", " << tangent_pts[i].y() << ")\n";
  std::cout << "\n";

  Circle d1(Point(0.0, 0.0), 3.0);
  Circle d2(Point(10.0, 0.0), 3.0);
  auto disjoint_pts = d1.IntersectionPoints(d2);
  std::cout << "Disjoint circles: " << disjoint_pts.size() << " intersection(s)\n\n";
}

static void DemonstrateTransformations() {
  std::cout << "=== Transformations ===\n\n";

  Circle c(Point(1.0, 1.0), 2.0);
  std::cout << "Original: " << c << "\n";
  c.Translate(Point(2.0, -1.0));
  std::cout << "After Translate(2, -1): " << c << "\n";
  c.Scale(0.5);
  std::cout << "After Scale(0.5): " << c << "\n";
  std::cout << "  Expected center (3, 0), radius 1\n\n";
}

int main() {
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "\n   Circle Class – Usage Examples (Group-13)\n\n";

  DemonstrateConstruction();
  DemonstrateContainment();
  DemonstrateOverlapAndSeparation();
  DemonstrateDistances();
  DemonstrateCircleContainment();
  DemonstrateIntersections();
  DemonstrateTransformations();

  std::cout << "All examples completed successfully.\n\n";
  return 0;
}
