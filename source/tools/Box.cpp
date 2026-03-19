/**
 * @file Box.cpp
 * @brief Implementation of the Box class
 *
 * @author Ebenezer Appiah (Group 13 - Math-Heavy Simulation World)
 */

#include "Box.hpp"

#include <limits>

namespace cse498 {

// Constructors

Box::Box()
    : center_x_(0.0), center_y_(0.0), half_width_(1.0), half_height_(1.0) {}

Box::Box(double center_x, double center_y, double half_width,
         double half_height)
    : center_x_(center_x),
      center_y_(center_y),
      half_width_(half_width),
      half_height_(half_height) {
  assert(half_width > 0.0 && "Box half_width must be positive");
  assert(half_height > 0.0 && "Box half_height must be positive");
  assert(std::isfinite(center_x) && "Box center_x must be finite");
  assert(std::isfinite(center_y) && "Box center_y must be finite");
  assert(std::isfinite(half_width) && "Box half_width must be finite");
  assert(std::isfinite(half_height) && "Box half_height must be finite");
}

Box::Box(const Point& center, double half_width, double half_height)
    : Box(center.getX(), center.getY(), half_width, half_height) {}

Box Box::FromCorners(const Point& corner1, const Point& corner2) {
  double min_x = std::min(corner1.getX(), corner2.getX());
  double max_x = std::max(corner1.getX(), corner2.getX());
  double min_y = std::min(corner1.getY(), corner2.getY());
  double max_y = std::max(corner1.getY(), corner2.getY());

  double center_x = (min_x + max_x) / 2.0;
  double center_y = (min_y + max_y) / 2.0;
  double half_width = (max_x - min_x) / 2.0;
  double half_height = (max_y - min_y) / 2.0;

  if (half_width < 1e-10) half_width = 1e-10;
  if (half_height < 1e-10) half_height = 1e-10;

  return Box(center_x, center_y, half_width, half_height);
}

Box Box::FromBottomLeft(const Point& bottom_left, double width, double height) {
  assert(width > 0.0 && "Box width must be positive");
  assert(height > 0.0 && "Box height must be positive");

  double center_x = bottom_left.getX() + width / 2.0;
  double center_y = bottom_left.getY() + height / 2.0;
  double half_width = width / 2.0;
  double half_height = height / 2.0;

  return Box(center_x, center_y, half_width, half_height);
}

// Position getters

Point Box::GetCenter() const { return Point(center_x_, center_y_); }

Point Box::GetBottomLeft() const {
  return Point(center_x_ - half_width_, center_y_ - half_height_);
}

Point Box::GetTopRight() const {
  return Point(center_x_ + half_width_, center_y_ + half_height_);
}

Point Box::GetBottomRight() const {
  return Point(center_x_ + half_width_, center_y_ - half_height_);
}

Point Box::GetTopLeft() const {
  return Point(center_x_ - half_width_, center_y_ + half_height_);
}

// Position setters

void Box::SetCenter(double x, double y) {
  assert(std::isfinite(x) && "Center x must be finite");
  assert(std::isfinite(y) && "Center y must be finite");
  center_x_ = x;
  center_y_ = y;
}

void Box::SetCenter(const Point& center) {
  SetCenter(center.getX(), center.getY());
}

void Box::SetBottomLeft(double x, double y) {
  assert(std::isfinite(x) && "Bottom-left x must be finite");
  assert(std::isfinite(y) && "Bottom-left y must be finite");
  center_x_ = x + half_width_;
  center_y_ = y + half_height_;
}

void Box::SetBottomLeft(const Point& bottom_left) {
  SetBottomLeft(bottom_left.getX(), bottom_left.getY());
}

// Dimension setters

void Box::SetHalfDimensions(double half_width, double half_height) {
  assert(half_width > 0.0 && "Half width must be positive");
  assert(half_height > 0.0 && "Half height must be positive");
  assert(std::isfinite(half_width) && "Half width must be finite");
  assert(std::isfinite(half_height) && "Half height must be finite");
  half_width_ = half_width;
  half_height_ = half_height;
}

void Box::SetDimensions(double width, double height) {
  assert(width > 0.0 && "Width must be positive");
  assert(height > 0.0 && "Height must be positive");
  SetHalfDimensions(width / 2.0, height / 2.0);
}

// Transformations

Box& Box::Translate(double dx, double dy) {
  assert(std::isfinite(dx) && "Translation dx must be finite");
  assert(std::isfinite(dy) && "Translation dy must be finite");
  center_x_ += dx;
  center_y_ += dy;
  return *this;
}

Box& Box::Translate(const Point& displacement) {
  return Translate(displacement.getX(), displacement.getY());
}

Box& Box::ScaleFromCenter(double scale_factor) {
  assert(scale_factor > 0.0 && "Scale factor must be positive");
  assert(std::isfinite(scale_factor) && "Scale factor must be finite");
  half_width_ *= scale_factor;
  half_height_ *= scale_factor;
  return *this;
}

Box& Box::ScaleFromCenter(double scale_x, double scale_y) {
  assert(scale_x > 0.0 && "Scale factor X must be positive");
  assert(scale_y > 0.0 && "Scale factor Y must be positive");
  assert(std::isfinite(scale_x) && "Scale factor X must be finite");
  assert(std::isfinite(scale_y) && "Scale factor Y must be finite");
  half_width_ *= scale_x;
  half_height_ *= scale_y;
  return *this;
}

Box& Box::ScaleFromPoint(double scale_factor, double origin_x,
                         double origin_y) {
  assert(scale_factor > 0.0 && "Scale factor must be positive");
  assert(std::isfinite(scale_factor) && "Scale factor must be finite");
  assert(std::isfinite(origin_x) && "Origin x must be finite");
  assert(std::isfinite(origin_y) && "Origin y must be finite");

  half_width_ *= scale_factor;
  half_height_ *= scale_factor;
  center_x_ = origin_x + (center_x_ - origin_x) * scale_factor;
  center_y_ = origin_y + (center_y_ - origin_y) * scale_factor;
  return *this;
}

Box& Box::ScaleFromPoint(double scale_factor, const Point& origin) {
  return ScaleFromPoint(scale_factor, origin.getX(), origin.getY());
}

Box& Box::Expand(double amount) {
  double new_half_width = half_width_ + amount;
  double new_half_height = half_height_ + amount;
  assert(new_half_width > 0.0 &&
         "Expansion would result in non-positive width");
  assert(new_half_height > 0.0 &&
         "Expansion would result in non-positive height");
  half_width_ = new_half_width;
  half_height_ = new_half_height;
  return *this;
}

Box& Box::Expand(double horizontal_amount, double vertical_amount) {
  double new_half_width = half_width_ + horizontal_amount;
  double new_half_height = half_height_ + vertical_amount;
  assert(new_half_width > 0.0 &&
         "Expansion would result in non-positive width");
  assert(new_half_height > 0.0 &&
         "Expansion would result in non-positive height");
  half_width_ = new_half_width;
  half_height_ = new_half_height;
  return *this;
}

// Collision detection

bool Box::Contains(double x, double y) const {
  return (x >= center_x_ - half_width_) && (x <= center_x_ + half_width_) &&
         (y >= center_y_ - half_height_) && (y <= center_y_ + half_height_);
}

bool Box::Contains(const Point& point) const {
  return Contains(point.getX(), point.getY());
}

bool Box::Overlaps(const Box& other) const {
  bool x_overlap =
      (center_x_ - half_width_ <= other.center_x_ + other.half_width_) &&
      (center_x_ + half_width_ >= other.center_x_ - other.half_width_);

  bool y_overlap =
      (center_y_ - half_height_ <= other.center_y_ + other.half_height_) &&
      (center_y_ + half_height_ >= other.center_y_ - other.half_height_);

  return x_overlap && y_overlap;
}

bool Box::Overlaps(const Circle& circle) const {
  Point closest = ClosestPointTo(circle.GetCenter());
  Point center = circle.GetCenter();

  double dx = closest.getX() - center.getX();
  double dy = closest.getY() - center.getY();
  double distance_squared = dx * dx + dy * dy;
  double radius = circle.GetRadius();

  return distance_squared <= radius * radius;
}

bool Box::CompletelyContains(const Box& other) const {
  double other_min_x = other.center_x_ - other.half_width_;
  double other_max_x = other.center_x_ + other.half_width_;
  double other_min_y = other.center_y_ - other.half_height_;
  double other_max_y = other.center_y_ + other.half_height_;

  double this_min_x = center_x_ - half_width_;
  double this_max_x = center_x_ + half_width_;
  double this_min_y = center_y_ - half_height_;
  double this_max_y = center_y_ + half_height_;

  return (other_min_x >= this_min_x) && (other_max_x <= this_max_x) &&
         (other_min_y >= this_min_y) && (other_max_y <= this_max_y);
}

std::optional<Box> Box::GetIntersection(const Box& other) const {
  if (!Overlaps(other)) {
    return std::nullopt;
  }

  double min_x =
      std::max(center_x_ - half_width_, other.center_x_ - other.half_width_);
  double max_x =
      std::min(center_x_ + half_width_, other.center_x_ + other.half_width_);
  double min_y =
      std::max(center_y_ - half_height_, other.center_y_ - other.half_height_);
  double max_y =
      std::min(center_y_ + half_height_, other.center_y_ + other.half_height_);

  double new_center_x = (min_x + max_x) / 2.0;
  double new_center_y = (min_y + max_y) / 2.0;
  double new_half_width = (max_x - min_x) / 2.0;
  double new_half_height = (max_y - min_y) / 2.0;

  return Box(new_center_x, new_center_y, new_half_width, new_half_height);
}

// Distance calculations

double Box::DistanceToPoint(double x, double y) const {
  Point closest = ClosestPointTo(x, y);
  double dx = x - closest.getX();
  double dy = y - closest.getY();
  return std::sqrt(dx * dx + dy * dy);
}

double Box::DistanceToPoint(const Point& point) const {
  return DistanceToPoint(point.getX(), point.getY());
}

Point Box::ClosestPointTo(double x, double y) const {
  double closest_x =
      std::clamp(x, center_x_ - half_width_, center_x_ + half_width_);
  double closest_y =
      std::clamp(y, center_y_ - half_height_, center_y_ + half_height_);
  return Point(closest_x, closest_y);
}

Point Box::ClosestPointTo(const Point& point) const {
  return ClosestPointTo(point.getX(), point.getY());
}

double Box::DistanceToBox(const Box& other) const {
  if (Overlaps(other)) {
    return 0.0;
  }

  Point closest_on_this = ClosestPointTo(other.GetCenter());
  return other.DistanceToPoint(closest_on_this);
}

// Geometric queries

double Box::GetDiagonalLength() const {
  double width = GetWidth();
  double height = GetHeight();
  return std::sqrt(width * width + height * height);
}

double Box::GetAspectRatio() const {
  assert(half_height_ > 0.0 && "Cannot compute aspect ratio with zero height");
  return GetWidth() / GetHeight();
}

bool Box::IsSquare(double tolerance) const {
  return std::abs(half_width_ - half_height_) <= tolerance;
}

Box Box::GetBoundingBox(const Box& other) const {
  double min_x =
      std::min(center_x_ - half_width_, other.center_x_ - other.half_width_);
  double max_x =
      std::max(center_x_ + half_width_, other.center_x_ + other.half_width_);
  double min_y =
      std::min(center_y_ - half_height_, other.center_y_ - other.half_height_);
  double max_y =
      std::max(center_y_ + half_height_, other.center_y_ + other.half_height_);

  double new_center_x = (min_x + max_x) / 2.0;
  double new_center_y = (min_y + max_y) / 2.0;
  double new_half_width = (max_x - min_x) / 2.0;
  double new_half_height = (max_y - min_y) / 2.0;

  return Box(new_center_x, new_center_y, new_half_width, new_half_height);
}

std::array<Point, 4> Box::GetCorners() const {
  return {GetBottomLeft(), GetBottomRight(), GetTopRight(), GetTopLeft()};
}

// Operators

bool Box::operator==(const Box& other) const {
  constexpr double epsilon = 1e-9;
  return (std::abs(center_x_ - other.center_x_) < epsilon) &&
         (std::abs(center_y_ - other.center_y_) < epsilon) &&
         (std::abs(half_width_ - other.half_width_) < epsilon) &&
         (std::abs(half_height_ - other.half_height_) < epsilon);
}

bool Box::operator!=(const Box& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& os, const Box& box) {
  os << "Box(center: (" << box.center_x_ << ", " << box.center_y_
     << "), half_dims: (" << box.half_width_ << ", " << box.half_height_
     << "))";
  return os;
}

// Validation

bool Box::IsValid() const {
  return (half_width_ > 0.0) && (half_height_ > 0.0) &&
         std::isfinite(center_x_) && std::isfinite(center_y_) &&
         std::isfinite(half_width_) && std::isfinite(half_height_);
}

}  // namespace cse498
