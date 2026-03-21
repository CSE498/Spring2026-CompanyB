#pragma once

#include <algorithm>  // for std::max
#include <cmath>      // for std::abs, std::sqrt, std::cos, std::sin
#include <expected>
#include <functional>
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

  // disable assignment constructor
  // Point& operator=(const Point&) = delete;

  // parameterized constructor
  Point(double X, double Y) : x(X), y(Y) {}

  // getters to get value of x and y
  double getX() const { return x; }
  double getY() const { return y; }

  // setters to set value of x and y
  Point& setX(double x_value = 0.0) {
    x = x_value;
    return *this;
  }
  Point& setY(double y_value = 0.0) {
    y = y_value;
    return *this;
  }

  // operators
  // addition (+=)
  Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  // subtraction (-=)
  Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  // 3. Geometry: dot and cross product, magnitude, normalize(return unit
  // vector),
  [[nodiscard]] double dot(const Point& other) const {
    return ((x * other.getX()) + (y * other.getY()));
  }

  [[nodiscard]] double magnitude() const { return std::sqrt(x * x + y * y); }

  // 4. Transforms: rotate, scale
  Point& scale(double scalar) {
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
  Point& rotate(double deg,
                const Point& pivot = {0, 0},
                bool counter_clockwise = true) {
    // Skip rotation if angle is effectively zero
    // (within floating-point tolerance)
    if (tol_equal(deg, 0))
      return *this;
    if (!counter_clockwise)
      deg = -deg;

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
  [[nodiscard]] double cross_product(const Point& other) const {
    // Since x, y are private in other, we use getters! Wait, inside class Point
    // we can access private members of other! But since the user's code had
    // other.y, it works natively! But using getters is also fine.
    return (x * other.getY()) - (y * other.getX());
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

namespace std {
template <>
struct hash<cse498::Point> {
  std::size_t operator()(const cse498::Point& p) const noexcept {
    return std::hash<double>{}(p.getX()) ^ (std::hash<double>{}(p.getY()) << 1);
  }
};
}  // namespace std
