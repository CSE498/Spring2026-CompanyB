/**
 * @file Point.hpp
 * @brief Two-dimensional point and vector utility type.
 */

#pragma once

#include <algorithm>  // for std::max
#include <cmath>      // for std::abs, std::sqrt, std::cos, std::sin
#include <expected>
#include <iostream>
#include <numbers>  // for std::numbers::pi
#include <string>

namespace cse498 {

/// Absolute tolerance used by tol_equal().
constexpr double EPSILON = 1e-9;

/// Relative tolerance used by tol_equal().
constexpr double RELATIVE_TOLERANCE = 1e-12;

/**
 * @brief Compare two floating-point values with absolute and relative error.
 * @param a First value.
 * @param b Second value.
 * @return true if a and b are equal within the configured tolerances.
 */
inline bool tol_equal(double a, double b) {
  return std::abs(a - b) <=
         EPSILON + RELATIVE_TOLERANCE * std::max(std::abs(a), std::abs(b));
}

/**
 * @brief Two-dimensional point and vector value.
 *
 * Point stores x and y coordinates as doubles and provides common vector math,
 * transforms, interpolation, and arithmetic operators used by the Group 13
 * geometry and infectious-world code.
 */
class Point {
 private:
  /// X coordinate.
  double x{0.0};

  /// Y coordinate.
  double y{0.0};

 public:
  /**
   * @brief Construct the origin point (0, 0).
   */
  Point() = default;

  /**
   * @brief Construct a point from x and y coordinates.
   * @param X X coordinate.
   * @param Y Y coordinate.
   */
  Point(double X, double Y) : x(X), y(Y) {}

  /**
   * @brief Get the x coordinate.
   * @return X coordinate.
   */
  constexpr double getX() const { return x; }

  /**
   * @brief Get the y coordinate.
   * @return Y coordinate.
   */
  constexpr double getY() const { return y; }

  /**
   * @brief Set the x coordinate.
   * @param x_value New x coordinate.
   * @return Reference to this point for chaining.
   */
  Point& setX(double x_value) {
    x = x_value;
    return *this;
  }

  /**
   * @brief Set the y coordinate.
   * @param y_value New y coordinate.
   * @return Reference to this point for chaining.
   */
  Point& setY(double y_value) {
    y = y_value;
    return *this;
  }

  /**
   * @brief Add another point component-wise.
   * @param other Point whose coordinates will be added.
   * @return Reference to this point.
   */
  constexpr Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  /**
   * @brief Subtract another point component-wise.
   * @param other Point whose coordinates will be subtracted.
   * @return Reference to this point.
   */
  constexpr Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  /**
   * @brief Compute the dot product with another point.
   * @param other Other vector.
   * @return Scalar dot product.
   */
  [[nodiscard]] constexpr double dot(const Point& other) const {
    return ((x * other.getX()) + (y * other.getY()));
  }

  /**
   * @brief Compute Euclidean vector magnitude.
   * @return Length from origin to this point.
   */
  [[nodiscard]] double magnitude() const { return std::sqrt(x * x + y * y); }

  /**
   * @brief Scale this point in place.
   * @param scalar Scalar multiplier.
   * @return Reference to this point.
   */
  constexpr Point& scale(double scalar) {
    x = x * scalar;
    y = y * scalar;
    return *this;
  }

  /**
   * @brief Create a scaled copy without mutating this point.
   * @param scalar Scalar multiplier.
   * @return Scaled point.
   */
  [[nodiscard]] Point scaled(double scalar) const {
    Point copy = *this;
    return copy.scale(scalar);
  }

  /**
   * @brief Normalize a point treated as a vector from the origin.
   * @param p Point/vector to normalize.
   * @return Unit vector, or an error string for an effectively zero vector.
   */
  [[nodiscard]] static std::expected<Point, std::string> normalize(
      const Point& p) {
    double mag = p.magnitude();
    if (tol_equal(mag, 0.0))
      return std::unexpected("Cannot normalize a zero vector");
    return Point(p.getX() / mag, p.getY() / mag);
  }

  /**
   * @brief Rotate this point around a pivot.
   * @param deg Rotation angle in degrees.
   * @param pivot Pivot point; defaults to the origin.
   * @param counter_clockwise true for counterclockwise, false for clockwise.
   * @return Reference to this point.
   */
  Point& rotate(double deg, const Point& pivot = {0, 0},
                bool counter_clockwise = true) {
    // Skip rotation if angle is effectively zero
    // (within floating-point tolerance)
    if (tol_equal(deg, 0)) return *this;
    if (!counter_clockwise) deg = -deg;

    double rad = deg * (std::numbers::pi / 180.0);

    double tempx = x - pivot.x;
    double tempy = y - pivot.y;

    double cos_rad = std::cos(rad);
    double sin_rad = std::sin(rad);

    x = ((tempx * cos_rad) - (tempy * sin_rad)) + pivot.x;
    y = ((tempx * sin_rad) + (tempy * cos_rad)) + pivot.y;

    return *this;
  }

  /**
   * @brief Create a rotated copy without mutating this point.
   * @param deg Rotation angle in degrees.
   * @param pivot Pivot point; defaults to the origin.
   * @param ccw true for counterclockwise, false for clockwise.
   * @return Rotated point.
   */
  [[nodiscard]] Point rotated(double deg, const Point& pivot = {0, 0},
                              bool ccw = true) const {
    Point copy = *this;
    return copy.rotate(deg, pivot, ccw);
  }

  /**
   * @brief Compute the 2-D cross product as a scalar.
   * @param other Other vector.
   * @return Scalar z-component of the 3-D cross product.
   */
  [[nodiscard]] constexpr double cross_product(const Point& other) const {
    return (x * other.y) - (y * other.x);
  }

  /**
   * @brief Compute squared vector length.
   * @return x*x + y*y.
   */
  [[nodiscard]] constexpr double lengthSq() const { return x * x + y * y; }

  /**
   * @brief Compute Euclidean distance to another point.
   * @param other Other point.
   * @return Distance between the points.
   */
  [[nodiscard]] double distanceTo(const Point& other) const {
    return std::sqrt(distanceSqTo(other));
  }

  /**
   * @brief Compute squared Euclidean distance to another point.
   * @param other Other point.
   * @return Squared distance between the points.
   */
  [[nodiscard]] constexpr double distanceSqTo(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return dx * dx + dy * dy;
  }

  /**
   * @brief Create a translated copy of this point.
   * @param delta Translation delta.
   * @return Translated point.
   */
  [[nodiscard]] Point translated(const Point& delta) const {
    return Point(x + delta.x, y + delta.y);
  }

  /**
   * @brief Return the left-hand perpendicular vector.
   * @return Point(-y, x).
   */
  [[nodiscard]] Point perpendicular() const { return Point(-y, x); }

  /**
   * @brief Compute this vector's angle from the positive x-axis.
   * @return Angle in radians from std::atan2(y, x).
   */
  [[nodiscard]] double angle() const { return std::atan2(y, x); }

  /**
   * @brief Linearly interpolate from this point to another.
   * @param other Endpoint.
   * @param t Interpolation factor.
   * @return Interpolated point.
   */
  [[nodiscard]] Point lerp(const Point& other, double t) const {
    return Point(x + (other.x - x) * t, y + (other.y - y) * t);
  }

  /**
   * @brief Construct a point from polar coordinates.
   * @param r Radius.
   * @param theta Angle in radians.
   * @return Cartesian point.
   */
  static Point fromPolar(double r, double theta) {
    return Point(r * std::cos(theta), r * std::sin(theta));
  }
};

/**
 * @brief Stream a point as `(x, y)`.
 * @param os Output stream.
 * @param p Point to print.
 * @return Reference to os.
 */
inline std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "(" << p.getX() << ", " << p.getY() << ")";
}

/// @brief Add two points component-wise.
Point operator+(const Point& lhs, const Point& rhs);
/// @brief Add a scalar to both point coordinates.
Point operator+(const Point& lhs, double rhs);
/// @brief Add a scalar to both point coordinates.
Point operator+(double lhs, const Point& rhs);

/// @brief Subtract two points component-wise.
Point operator-(const Point& lhs, const Point& rhs);
/// @brief Subtract a scalar from both point coordinates.
Point operator-(const Point& lhs, double rhs);
/// @brief Subtract point coordinates from a scalar.
Point operator-(double lhs, const Point& rhs);

/// @brief Multiply two points component-wise.
Point operator*(const Point& lhs, const Point& rhs);
/// @brief Multiply both point coordinates by a scalar.
Point operator*(const Point& lhs, double rhs);
/// @brief Multiply both point coordinates by a scalar.
Point operator*(double lhs, const Point& rhs);

/// @brief Compare two points with tolerant coordinate equality.
bool operator==(const Point& lhs, const Point& rhs);
/// @brief Compare both point coordinates to a scalar.
bool operator==(const Point& lhs, double rhs);
/// @brief Compare both point coordinates to a scalar.
bool operator==(double lhs, const Point& rhs);

/// @brief Compare two points with tolerant coordinate inequality.
bool operator!=(const Point& lhs, const Point& rhs);
/// @brief Compare point coordinates to a scalar for inequality.
bool operator!=(const Point& lhs, double rhs);
/// @brief Compare point coordinates to a scalar for inequality.
bool operator!=(double lhs, const Point& rhs);

/// @brief Free-function dot product.
double dot(const Point& A, const Point& B);

}  // namespace cse498
