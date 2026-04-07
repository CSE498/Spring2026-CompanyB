// PointClass.hpp
#pragma once

#include <algorithm>  // for std::max
#include <cmath>      // for std::abs, std::sqrt, std::cos, std::sin
#include <expected>
#include <iostream>
#include <numbers>  // for std::numbers::pi
#include <string>

namespace cse498 {

// this block creates a function to safely compare doubles
constexpr double EPSILON = 1e-9;
constexpr double RELATIVE_TOLERANCE = 1e-12;
inline bool tol_equal(double a, double b) {
  return std::abs(a - b) <=
         EPSILON + RELATIVE_TOLERANCE * std::max(std::abs(a), std::abs(b));
}

class Point {
 private:
  double x{}, y{};

 public:
  // default constructor
  Point() : x(0.0), y(0.0) {}

  // parameterized constructor
  Point(double X, double Y) : x(X), y(Y) {}

  // getters to get value of x and y
  constexpr double getX() const { return x; }
  constexpr double getY() const { return y; }

  // setters to set value of x and y
  Point& setX(double x_value) {
    x = x_value;
    return *this;
  }
  Point& setY(double y_value) {
    y = y_value;
    return *this;
  }

  // operators
  // addition (+=)
  constexpr Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  // subtraction (-=)
  constexpr Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  // 3. Geometry: dot and cross product, magnitude, normalize(return unit
  // vector),
  [[nodiscard]] constexpr double dot(const Point& other) const {
    return ((x * other.getX()) + (y * other.getY()));
  }

  [[nodiscard]] double magnitude() const { return std::sqrt(x * x + y * y); }

  // 4. Transforms: rotate, scale
  constexpr Point& scale(double scalar) {
    x = x * scalar;
    y = y * scalar;
    return *this;
  }

  // normalize
  Point& normalize() {
    double mag = this->magnitude();
    if (!tol_equal(mag, 0.0)) {
      x = x / mag;
      y = y / mag;
    }
    return *this;
  }

  [[nodiscard]] static std::expected<Point, std::string> safe_normalize(
      const Point& p) {
    double mag = p.magnitude();
    if (tol_equal(mag, 0.0))
      return std::unexpected("Cannot normalize a zero vector");
    return Point(p.getX() / mag, p.getY() / mag);
  }

  // rotate
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

  // 2D cross product -- useful for finding area of the parallelogram
  // area of triangle, possibly torque
  // 2D cross product yields a fake "k" in ijk system
  // my method will return a scalar, not a point vector
  [[nodiscard]] constexpr double cross_product(const Point& other) const {
    return (x * other.y) - (y * other.x);
  }

  // Additional geometric operations

  // Squared length.
  [[nodiscard]] constexpr double lengthSq() const { return x * x + y * y; }

  // Distance to another point.
  [[nodiscard]] double distanceTo(const Point& other) const {
    return std::sqrt(distanceSqTo(other));
  }

  // Squared distance to another point.
  [[nodiscard]] double distanceSqTo(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return dx * dx + dy * dy;
  }

  // Return a new point translate.
  [[nodiscard]] Point translated(const Point& delta) const {
    return Point(x + delta.x, y + delta.y);
  }

  // Return a perpendicular vector.
  [[nodiscard]] Point perpendicular() const { return Point(-y, x); }

  // Angle of the vector.
  [[nodiscard]] double angle() const { return std::atan2(y, x); }

  // Linear interpolation.
  [[nodiscard]] Point lerp(const Point& other, double t) const {
    return Point(x + (other.x - x) * t, y + (other.y - y) * t);
  }

  // Construct a point from polar coordinates.
  static Point fromPolar(double r, double theta) {
    return Point(r * std::cos(theta), r * std::sin(theta));
  }
};

inline std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "(" << p.getX() << ", " << p.getY() << ")";
}

// The addition "operators"
Point operator+(const Point& lhs, const Point& rhs);
Point operator+(const Point& lhs, double rhs);
Point operator+(double lhs, const Point& rhs);

// The subtraction "operators"
Point operator-(const Point& lhs, const Point& rhs);
Point operator-(const Point& lhs, double rhs);
Point operator-(double lhs, const Point& rhs);

// The product "operators"
Point operator*(const Point& lhs, const Point& rhs);
Point operator*(const Point& lhs, double rhs);
Point operator*(double lhs, const Point& rhs);

// The equality "operators"
bool operator==(const Point& lhs, const Point& rhs);
bool operator==(const Point& lhs, double rhs);
bool operator==(double lhs, const Point& rhs);

// The unequal "operators"
bool operator!=(const Point& lhs, const Point& rhs);
bool operator!=(const Point& lhs, double rhs);
bool operator!=(double lhs, const Point& rhs);

// dot product
double dot(const Point& A, const Point& B);

}  // namespace cse498
