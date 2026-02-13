#pragma once

// Temporary 'stub' for until Group 13 provides their Point.hpp

#include <cstddef>
#include <functional>

namespace Math {

struct Point {
  double x = 0.0;
  double y = 0.0;

  // Equality operators for use in containers
  bool operator==(const Point &other) const noexcept {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Point &other) const noexcept {
    return !(*this == other);
  }
};

// Hash function for Point to enable use in unordered containers
struct PointHash {
  std::size_t operator()(const Point &p) const noexcept {
    // Combine hashes of x and y coordinates
    std::size_t h1 = std::hash<double>{}(p.x);
    std::size_t h2 = std::hash<double>{}(p.y);
    return h1 ^ (h2 << 1);
  }
};

} // namespace Math
