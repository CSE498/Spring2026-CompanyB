// Temporary stub until Group 13 provides their Point.
// Interface mirrors Group 13's Point so the swap is seamless.

#pragma once

#include <cstddef>
#include <functional>

namespace Math {

class Point {
public:
  Point() : x_(0.0), y_(0.0) {}
  Point(double x, double y) : x_(x), y_(y) {}

  double x() const { return x_; }
  double y() const { return y_; }

  Point operator+(const Point &other) const {
    return Point{x_ + other.x_, y_ + other.y_};
  }

  bool operator==(const Point &other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

private:
  double x_;
  double y_;
};

// Hash function for Point to enable use in unordered containers
struct PointHash {
  std::size_t operator()(const Point &p) const noexcept {
    // Combine hashes of x and y coordinates
    std::size_t h1 = std::hash<double>{}(p.x());
    std::size_t h2 = std::hash<double>{}(p.y());
    return h1 ^ (h2 << 1);
  }
};

} // namespace Math

// Alias for backward compatibility
using Point = Math::Point;
