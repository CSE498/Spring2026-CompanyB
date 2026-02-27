/**
 * @file Circle.hpp
 * @brief Lightweight circle geometry type for 2D simulations.
 * @author Group-13 (Lemuel). Developed with AI assistance (Cursor).
 *
 * Assumes an existing Point type with:
 *   - double getX() const, double getY() const
 *   - arithmetic operators (e.g., Point + Point)
 *   - equality operator
 *
 * Company shared C++ classes live in source/tools/; headers use .hpp.
 */

#pragma once

#include <cstddef>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <vector>

#if __has_include(<numbers>)
#include <numbers>
#endif

#include "PointClass.hpp"

namespace cse498 {

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
  static constexpr double EPS = 1e-9;  ///< Default tolerance for floating-point comparisons
#if defined(__cpp_lib_math_constants)
  static constexpr double PI = std::numbers::pi_v<double>;  ///< Pi constant
#else
  static constexpr double PI = 3.14159265358979323846;  ///< Pi constant
#endif
  static constexpr double kEps = EPS;  ///< Legacy alias for compatibility
  static constexpr double kPi = PI;    ///< Legacy alias for compatibility

  /**
   * @brief Default constructor: center at origin, radius zero.
   */
  Circle() : center_(Point{}), radius_(0.0) {}

  /**
   * @brief Construct circle from center and radius.
   * @param center Center of the circle
   * @param radius Non-negative radius
   */
  Circle(const Point& center, double radius) : center_(center), radius_(radius) {
    if (radius_ < 0.0) {
      throw std::invalid_argument("Circle radius must be non-negative.");
    }
  }


  /**
   * @brief Get the center of the circle.
   * @return Const reference to the center point
   */
  [[nodiscard]] const Point& GetCenter() const { return center_; }

  /**
   * @brief Set the center of the circle.
   * @param center New center point
   */
  void SetCenter(const Point& center) { center_ = center; }

  /**
   * @brief Get the radius of the circle.
   * @return Radius (>= 0)
   */
  [[nodiscard]] double GetRadius() const { return radius_; }

  /**
   * @brief Set the radius of the circle.
   * @param radius Non-negative radius
   */
  void SetRadius(double radius) {
    if (radius < 0.0) {
      throw std::invalid_argument("Circle radius must be non-negative.");
    }
    radius_ = radius;
  }


  /**
   * @brief Test whether a point lies inside or on the circle.
   * @param p Point to test
   * @return true if distance from center to p <= radius (within EPS)
   */
  [[nodiscard]] bool Contains(const Point& p) const {
    const double dist2 = DistanceSquared(center_, p);
    const double r = radius_ + EPS;
    return dist2 <= r * r;
  }

  /**
   * @brief Test whether another circle lies entirely inside this circle.
   * @param other Circle to test
   * @return true if other's center plus its radius <= this radius (within EPS)
   */
  [[nodiscard]] bool Contains(const Circle& other) const {
    const double center_dist = CenterDistanceTo(other);
    return center_dist + other.radius_ <= radius_ + EPS;
  }

  /**
   * @brief Test whether a point lies on the circle boundary.
   * @param p Point to test
   * @param eps Tolerance for |distance - radius| (default: EPS)
   * @return true if distance from center to p equals radius within eps
   */
  [[nodiscard]] bool OnBoundary(const Point& p, double eps = EPS) const {
    const double dist = DistanceTo(p);
    return std::fabs(dist - radius_) <= eps;
  }

  /**
   * @brief Signed distance from a point to the circle boundary.
   * @param p Point to evaluate
   * @return distance(p, center) - radius (negative inside, zero on boundary, positive outside)
   */
  [[nodiscard]] double SignedDistanceToBoundary(const Point& p) const {
    return DistanceTo(p) - radius_;
  }

  /**
   * @brief Distance from circle center to a point.
   * @param p Point to measure to
   * @return Euclidean distance from center to p
   */
  [[nodiscard]] double DistanceTo(const Point& p) const {
    return std::sqrt(DistanceSquared(center_, p));
  }

  /**
   * @brief Distance between this circle's center and another circle's center.
   * @param other Other circle
   * @return Euclidean distance between centers
   */
  [[nodiscard]] double CenterDistanceTo(const Circle& other) const {
    return std::sqrt(DistanceSquared(center_, other.center_));
  }

  /**
   * @brief Backward-compatible alias for center-to-center distance.
   * @param other Other circle
   * @return Euclidean distance between centers
   */
  [[nodiscard]] double DistanceTo(const Circle& other) const {
    return CenterDistanceTo(other);
  }

  /**
   * @brief Test whether this circle overlaps another (tangency counts as overlap).
   * @param other Circle to test
   * @return true if distance between centers <= sum of radii (within EPS)
   */
  [[nodiscard]] bool Overlaps(const Circle& other) const {
    const double sum = radius_ + other.radius_ + EPS;
    const double dist2 = DistanceSquared(center_, other.center_);
    return dist2 <= sum * sum;
  }

  /**
   * @brief Test whether this circle and another are disjoint (no overlap).
   * @param other Circle to test
   * @return true if the circles do not overlap
   */
  [[nodiscard]] bool IsDisjoint(const Circle& other) const {
    return !Overlaps(other);
  }

  /**
   * @brief Test whether this circle is tangent to another (external or internal).
   * @param other Circle to test
   * @param eps Tolerance (default: EPS)
   * @return true if distance equals sum or |difference| of radii within eps
   */
  [[nodiscard]] bool IsTangentTo(const Circle& other, double eps = EPS) const {
    const double dist = CenterDistanceTo(other);
    const double ext_tangent = std::fabs(dist - (radius_ + other.radius_));
    const double int_tangent = std::fabs(dist - std::fabs(radius_ - other.radius_));
    return ext_tangent <= eps || int_tangent <= eps;
  }

  /** @brief Diameter (2 * radius). */
  [[nodiscard]] double Diameter() const { return 2.0 * radius_; }

  /** @brief Circumference (2 * pi * radius). */
  [[nodiscard]] double Circumference() const { return 2.0 * PI * radius_; }

  /** @brief Area (pi * radius^2). */
  [[nodiscard]] double Area() const { return PI * radius_ * radius_; }


  /**
   * @brief Compute intersection points of this circle with another.
   * @param other Other circle
   * @param eps Tolerance for degenerate cases (default: EPS)
   * @return Vector of 0, 1 (tangent), or 2 intersection points; empty for coincident circles
   */
  [[nodiscard]] std::vector<Point> IntersectionPoints(const Circle& other,
                                                      double eps = EPS) const {
    std::vector<Point> points;

    const double d = CenterDistanceTo(other);
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
    const double x0 = center_.getX();
    const double y0 = center_.getY();
    const double x1 = other.center_.getX();
    const double y1 = other.center_.getY();

    const double px = x0 + a * (x1 - x0) / d;
    const double py = y0 + a * (y1 - y0) / d;
    const double rx = -(y1 - y0) * (h / d);
    const double ry = (x1 - x0) * (h / d);

    points.emplace_back(px + rx, py + ry);
    if (h > eps) {
      points.emplace_back(px - rx, py - ry);
    }
    return points;
  }

  /**
   * @brief Translate the circle by a delta vector.
   * @param delta Point to add to the center
   */
  void Translate(const Point& delta) { center_ = center_ + delta; }

  /**
   * @brief Scale the radius by a non-negative factor.
   * @param factor Scale factor
   */
  void Scale(double factor) {
    if (factor < 0.0) {
      throw std::invalid_argument("Circle scale factor must be non-negative.");
    }
    radius_ *= factor;
  }

  /**
   * @brief Minimum distance between the two circles' boundaries.
   * @param other Other circle
   * @return max(0, center_distance - radius - other.radius)
   */
  [[nodiscard]] double SeparationDistance(const Circle& other) const {
    const double dist = CenterDistanceTo(other);
    const double sep = dist - (radius_ + other.radius_);
    return sep > 0.0 ? sep : 0.0;
  }

  /**
   * @brief Compare circles using tolerant floating-point equality.
   * @param other Circle to compare
   * @return true if centers and radii are equal within EPS
   */
  [[nodiscard]] bool operator==(const Circle& other) const {
    return DistanceSquared(center_, other.center_) <= (EPS * EPS) &&
           std::fabs(radius_ - other.radius_) <= EPS;
  }

  /**
   * @brief Compare circles for inequality.
   * @param other Circle to compare
   * @return true if circles are not equal within EPS
   */
  [[nodiscard]] bool operator!=(const Circle& other) const { return !(*this == other); }

 private:
  Point center_;   ///< Center of the circle
  double radius_;  ///< Non-negative radius

  /**
   * @brief Squared distance between two points (avoids sqrt when comparing).
   */
  [[nodiscard]] static double DistanceSquared(const Point& a, const Point& b) {
    const double dx = a.getX() - b.getX();
    const double dy = a.getY() - b.getY();
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
  os << "Circle(center=(" << c.GetCenter().getX() << ", " << c.GetCenter().getY()
     << "), radius=" << c.GetRadius() << ")";
  return os;
}

}  // namespace cse498
