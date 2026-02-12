#pragma once

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
