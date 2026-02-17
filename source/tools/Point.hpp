/**
 * @file Point.hpp
 * @brief 2D point type used by Circle and other geometry in source/tools.
 * @author Group-13 (Lemuel). Developed with AI assistance (Cursor).
 */

#pragma once

namespace cse498 {

class Point {
 public:
  Point() : x_(0.0), y_(0.0) {}
  Point(double x, double y) : x_(x), y_(y) {}

  double x() const { return x_; }
  double y() const { return y_; }

  Point operator+(const Point& other) const {
    return Point{x_ + other.x_, y_ + other.y_};
  }

  bool operator==(const Point& other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

 private:
  double x_;
  double y_;
};

}  // namespace cse498
