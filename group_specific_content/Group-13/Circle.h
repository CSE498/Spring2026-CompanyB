/**
 * @file Circle.h
 * @brief Lightweight circle geometry type for 2D simulations.
 *
 * Assumes an existing Point type with:
 *   - double x() const, double y() const
 *   - arithmetic operators (e.g., Point + Point)
 *   - equality operator
 *
 * If your Point header name differs, update the include below.
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>
#include <ostream>
#include <vector>

#include "Point.h"

/**
 * @class Circle
 * @brief 2D circle defined by center point and non-negative radius.
 *
 * Provides geometry queries (containment, distance, overlap), scalar properties
 * (diameter, circumference, area), circle-circle intersection points, and
 * transformations (translate, scale).
 *
 * Example usage:
 * @code
 *   Circle c(Point(0, 0), 2.0);
 *   if (c.Contains(Point(1, 1))) { ... }
 *   auto pts = c.IntersectionPoints(other);
 *   c.Translate(Point(1, 0));
 * @endcode
 */
class Circle {
 public:
  static constexpr double kEps = 1e-9;   ///< Default tolerance for floating-point comparisons
  static constexpr double kPi = 3.14159265358979323846;  ///< Pi constant

  // ---------------------------------------------------------------------------
  // Constructors
  // ---------------------------------------------------------------------------

  /**
   * @brief Default constructor: center at origin, radius zero.
   */
  Circle() : center_(Point{}), radius_(0.0) {}

  /**
   * @brief Construct circle from center and radius.
   * @param center Center of the circle
   * @param radius Non-negative radius (asserted >= 0)
   */
  Circle(const Point& center, double radius) : center_(center), radius_(radius) {
    assert(radius_ >= 0.0);
  }

  // ---------------------------------------------------------------------------
  // Accessors / Mutators
  // ---------------------------------------------------------------------------

  /**
   * @brief Get the center of the circle.
   * @return Const reference to the center point
   */
  const Point& GetCenter() const { return center_; }

  /**
   * @brief Set the center of the circle.
   * @param center New center point
   */
  void SetCenter(const Point& center) { center_ = center; }

  /**
   * @brief Get the radius of the circle.
   * @return Radius (>= 0)
   */
  double GetRadius() const { return radius_; }

  /**
   * @brief Set the radius of the circle.
   * @param radius Non-negative radius (asserted >= 0)
   */
  void SetRadius(double radius) {
    assert(radius >= 0.0);
    radius_ = radius;
  }

  // ---------------------------------------------------------------------------
  // Geometry Queries
  // ---------------------------------------------------------------------------

  /**
   * @brief Test whether a point lies inside or on the circle.
   * @param p Point to test
   * @return true if distance from center to p <= radius (within kEps)
   */
  bool Contains(const Point& p) const {
    const double dist2 = DistanceSquared(center_, p);
    const double r = radius_ + kEps;
    return dist2 <= r * r;
  }

  /**
   * @brief Test whether another circle lies entirely inside this circle.
   * @param other Circle to test
   * @return true if other's center plus its radius <= this radius (within kEps)
   */
  bool Contains(const Circle& other) const {
    const double center_dist = DistanceTo(other);
    return center_dist + other.radius_ <= radius_ + kEps;
  }

  /**
   * @brief Test whether a point lies on the circle boundary.
   * @param p Point to test
   * @param eps Tolerance for |distance - radius| (default: kEps)
   * @return true if distance from center to p equals radius within eps
   */
  bool OnBoundary(const Point& p, double eps = kEps) const {
    const double dist = DistanceTo(p);
    return std::fabs(dist - radius_) <= eps;
  }

  /**
   * @brief Signed distance from a point to the circle boundary.
   * @param p Point to evaluate
   * @return distance(p, center) - radius (negative inside, zero on boundary, positive outside)
   */
  double SignedDistanceToBoundary(const Point& p) const {
    return DistanceTo(p) - radius_;
  }

  /**
   * @brief Distance from circle center to a point.
   * @param p Point to measure to
   * @return Euclidean distance from center to p
   */
  double DistanceTo(const Point& p) const {
    return std::sqrt(DistanceSquared(center_, p));
  }

  /**
   * @brief Distance between this circle's center and another circle's center.
   * @param other Other circle
   * @return Euclidean distance between centers
   */
  double DistanceTo(const Circle& other) const {
    return std::sqrt(DistanceSquared(center_, other.center_));
  }

  // ---------------------------------------------------------------------------
  // Overlap Detection
  // ---------------------------------------------------------------------------

  /**
   * @brief Test whether this circle overlaps another (tangency counts as overlap).
   * @param other Circle to test
   * @return true if distance between centers <= sum of radii (within kEps)
   */
  bool Overlaps(const Circle& other) const {
    const double sum = radius_ + other.radius_ + kEps;
    const double dist2 = DistanceSquared(center_, other.center_);
    return dist2 <= sum * sum;
  }

  /**
   * @brief Test whether this circle and another are disjoint (no overlap).
   * @param other Circle to test
   * @return true if SeparationDistance(other) > 0
   */
  bool IsDisjoint(const Circle& other) const {
    return SeparationDistance(other) > 0.0;
  }

  /**
   * @brief Test whether this circle is tangent to another (external or internal).
   * @param other Circle to test
   * @param eps Tolerance (default: kEps)
   * @return true if distance equals sum or |difference| of radii within eps
   */
  bool IsTangentTo(const Circle& other, double eps = kEps) const {
    const double dist = DistanceTo(other);
    const double ext_tangent = std::fabs(dist - (radius_ + other.radius_));
    const double int_tangent = std::fabs(dist - std::fabs(radius_ - other.radius_));
    return ext_tangent <= eps || int_tangent <= eps;
  }

  // ---------------------------------------------------------------------------
  // Scalar Properties
  // ---------------------------------------------------------------------------

  /** @brief Diameter (2 * radius). */
  double Diameter() const { return 2.0 * radius_; }

  /** @brief Circumference (2 * pi * radius). */
  double Circumference() const { return 2.0 * kPi * radius_; }

  /** @brief Area (pi * radius^2). */
  double Area() const { return kPi * radius_ * radius_; }

  // ---------------------------------------------------------------------------
  // Intersection
  // ---------------------------------------------------------------------------

  /**
   * @brief Compute intersection points of this circle with another.
   * @param other Other circle
   * @param eps Tolerance for degenerate cases (default: kEps)
   * @return Vector of 0, 1 (tangent), or 2 intersection points; empty for coincident circles
   */
  std::vector<Point> IntersectionPoints(const Circle& other, double eps = kEps) const {
    std::vector<Point> points;

    const double d = DistanceTo(other);
    const double r0 = radius_;
    const double r1 = other.radius_;

    if (d <= eps && std::fabs(r0 - r1) <= eps) return points;
    if (d > r0 + r1 + eps) return points;
    if (d < std::fabs(r0 - r1) - eps) return points;
    if (d <= eps) return points;

    const double a = (r0 * r0 - r1 * r1 + d * d) / (2.0 * d);
    double h2 = r0 * r0 - a * a;
    if (h2 < 0.0 && std::fabs(h2) <= eps) h2 = 0.0;
    if (h2 < 0.0) return points;

    const double h = std::sqrt(h2);
    const double x0 = center_.x();
    const double y0 = center_.y();
    const double x1 = other.center_.x();
    const double y1 = other.center_.y();

    const double px = x0 + a * (x1 - x0) / d;
    const double py = y0 + a * (y1 - y0) / d;
    const double rx = -(y1 - y0) * (h / d);
    const double ry = (x1 - x0) * (h / d);

    points.emplace_back(px + rx, py + ry);
    if (h > eps) points.emplace_back(px - rx, py - ry);
    return points;
  }

  // ---------------------------------------------------------------------------
  // Transformations
  // ---------------------------------------------------------------------------

  /**
   * @brief Translate the circle by a delta vector.
   * @param delta Point to add to the center
   */
  void Translate(const Point& delta) { center_ = center_ + delta; }

  /**
   * @brief Scale the radius by a non-negative factor.
   * @param factor Scale factor (asserted >= 0)
   */
  void Scale(double factor) {
    assert(factor >= 0.0);
    radius_ *= factor;
  }

  // ---------------------------------------------------------------------------
  // Helpers
  // ---------------------------------------------------------------------------

  /**
   * @brief Minimum distance between the two circles' boundaries.
   * @param other Other circle
   * @return max(0, center_distance - radius - other.radius)
   */
  double SeparationDistance(const Circle& other) const {
    const double dist = DistanceTo(other);
    const double sep = dist - (radius_ + other.radius_);
    return sep > 0.0 ? sep : 0.0;
  }

 private:
  Point center_;   ///< Center of the circle
  double radius_;  ///< Non-negative radius

  /**
   * @brief Squared distance between two points (avoids sqrt when comparing).
   */
  static double DistanceSquared(const Point& a, const Point& b) {
    const double dx = a.x() - b.x();
    const double dy = a.y() - b.y();
    return dx * dx + dy * dy;
  }
};

/**
 * @brief Stream a circle as Circle(center=(x, y), radius=r).
 * @param os Output stream
 * @param c Circle to print
 * @return Reference to the stream
 */
inline std::ostream& operator<<(std::ostream& os, const Circle& c) {
  os << "Circle(center=(" << c.GetCenter().x() << ", " << c.GetCenter().y()
     << "), radius=" << c.GetRadius() << ")";
  return os;
}
