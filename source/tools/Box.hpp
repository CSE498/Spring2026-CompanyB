/**
 * @file Box.hpp
 * @brief Axis-Aligned Bounding Box (AABB) class for 2D geometric operations
 *
 * @author Ebenezer Appiah (Group 13 - Math-Heavy Simulation World)
 */

#ifndef CSE498_BOX_HPP
#define CSE498_BOX_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>

#include "Circle.hpp"
#include "Point.hpp"

namespace cse498 {

/**
 * @class Box
 * @brief Axis-aligned 2-D rectangular box.
 *
 * Box stores its center and half dimensions. Public helpers expose common
 * construction styles, transforms, overlap checks, distance queries, and
 * bounding geometry used by the infectious-world quarantine logic.
 */
class Box {
 private:
  /// X coordinate of the box center.
  double center_x_;

  /// Y coordinate of the box center.
  double center_y_;

  /// Half of the box width; must be positive.
  double half_width_;

  /// Half of the box height; must be positive.
  double half_height_;

 public:
  /**
   * @brief Construct a default 2x2 box centered at the origin.
   */
  Box();

  /**
   * @brief Construct a box from center coordinates and half dimensions.
   * @param center_x X coordinate of the center.
   * @param center_y Y coordinate of the center.
   * @param half_width Half of the width; must be positive.
   * @param half_height Half of the height; must be positive.
   */
  Box(double center_x, double center_y, double half_width, double half_height);

  /**
   * @brief Construct a box from a center point and half dimensions.
   * @param center Center point.
   * @param half_width Half of the width; must be positive.
   * @param half_height Half of the height; must be positive.
   */
  Box(const Point& center, double half_width, double half_height);

  /**
   * @brief Construct the smallest axis-aligned box spanning two corners.
   * @param corner1 First corner.
   * @param corner2 Opposite corner.
   * @return Box spanning both corners.
   */
  static Box FromCorners(const Point& corner1, const Point& corner2);

  /**
   * @brief Construct a box from bottom-left corner and full dimensions.
   * @param bottom_left Bottom-left corner.
   * @param width Full width; must be positive.
   * @param height Full height; must be positive.
   * @return Box with the requested bounds.
   */
  static Box FromBottomLeft(const Point& bottom_left, double width,
                            double height);

  /**
   * @brief Get the center point.
   * @return Center as a Point.
   */
  Point GetCenter() const;

  /// @brief Get center x coordinate.
  double GetCenterX() const { return center_x_; }

  /// @brief Get center y coordinate.
  double GetCenterY() const { return center_y_; }

  /// @brief Get the bottom-left corner.
  Point GetBottomLeft() const;

  /// @brief Get the top-right corner.
  Point GetTopRight() const;

  /// @brief Get the bottom-right corner.
  Point GetBottomRight() const;

  /// @brief Get the top-left corner.
  Point GetTopLeft() const;

  /// @brief Get half of the box width.
  double GetHalfWidth() const { return half_width_; }

  /// @brief Get half of the box height.
  double GetHalfHeight() const { return half_height_; }

  /// @brief Get full box width.
  double GetWidth() const { return 2.0 * half_width_; }

  /// @brief Get full box height.
  double GetHeight() const { return 2.0 * half_height_; }

  /// @brief Get rectangular area.
  double GetArea() const { return GetWidth() * GetHeight(); }

  /// @brief Get rectangular perimeter.
  double GetPerimeter() const { return 2.0 * (GetWidth() + GetHeight()); }

  /**
   * @brief Set center coordinates.
   * @param x New center x coordinate.
   * @param y New center y coordinate.
   */
  void SetCenter(double x, double y);

  /**
   * @brief Set center point.
   * @param center New center point.
   */
  void SetCenter(const Point& center);

  /**
   * @brief Move the box so its bottom-left corner is at (x, y).
   * @param x New bottom-left x coordinate.
   * @param y New bottom-left y coordinate.
   */
  void SetBottomLeft(double x, double y);

  /**
   * @brief Move the box so its bottom-left corner is at bottom_left.
   * @param bottom_left New bottom-left corner.
   */
  void SetBottomLeft(const Point& bottom_left);

  /**
   * @brief Set half dimensions directly.
   * @param half_width New half width; must be positive.
   * @param half_height New half height; must be positive.
   */
  void SetHalfDimensions(double half_width, double half_height);

  /**
   * @brief Set full dimensions.
   * @param width New full width; must be positive.
   * @param height New full height; must be positive.
   */
  void SetDimensions(double width, double height);

  /**
   * @brief Translate the box center.
   * @param dx Horizontal displacement.
   * @param dy Vertical displacement.
   * @return Reference to this box for chaining.
   */
  Box& Translate(double dx, double dy);

  /**
   * @brief Translate the box center by a point/vector.
   * @param displacement Translation vector.
   * @return Reference to this box for chaining.
   */
  Box& Translate(const Point& displacement);

  /**
   * @brief Uniformly scale the box about its center.
   * @param scale_factor Positive scale factor.
   * @return Reference to this box for chaining.
   */
  Box& ScaleFromCenter(double scale_factor);

  /**
   * @brief Non-uniformly scale the box about its center.
   * @param scale_x Positive horizontal scale factor.
   * @param scale_y Positive vertical scale factor.
   * @return Reference to this box for chaining.
   */
  Box& ScaleFromCenter(double scale_x, double scale_y);

  /**
   * @brief Uniformly scale the box about an arbitrary origin.
   * @param scale_factor Positive scale factor.
   * @param origin_x Origin x coordinate.
   * @param origin_y Origin y coordinate.
   * @return Reference to this box for chaining.
   */
  Box& ScaleFromPoint(double scale_factor, double origin_x, double origin_y);

  /**
   * @brief Uniformly scale the box about an arbitrary origin.
   * @param scale_factor Positive scale factor.
   * @param origin Origin point.
   * @return Reference to this box for chaining.
   */
  Box& ScaleFromPoint(double scale_factor, const Point& origin);

  /**
   * @brief Expands the box by adding 'amount' to each half-dimension
   * @param amount Amount to add (positive = grow, negative = shrink)
   * @return Reference to this box for method chaining
   *
   * Example: Expand(1.0) on a 4x6 box makes it 6x8
   */
  Box& Expand(double amount);

  /**
   * @brief Expands the box non-uniformly
   * @param horizontal_amount Amount to add to half-width
   * @param vertical_amount Amount to add to half-height
   * @return Reference to this box for method chaining
   */
  Box& Expand(double horizontal_amount, double vertical_amount);

  /**
   * @brief Check whether the box contains a coordinate.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return true if the point is inside or on the boundary.
   */
  bool Contains(double x, double y) const;

  /**
   * @brief Check whether the box contains a point.
   * @param point Point to test.
   * @return true if the point is inside or on the boundary.
   */
  bool Contains(const Point& point) const;

  /**
   * @brief Check whether two boxes overlap.
   * @param other Other box.
   * @return true if the boxes overlap or touch.
   */
  bool Overlaps(const Box& other) const;

  /**
   * @brief Check whether this box overlaps a circle.
   * @param circle Circle to test.
   * @return true if the circle overlaps or touches the box.
   */
  bool Overlaps(const Circle& circle) const;

  /**
   * @brief Check whether this box fully contains another box.
   * @param other Box to test.
   * @return true if all of other lies inside this box.
   */
  bool CompletelyContains(const Box& other) const;

  /**
   * @brief Compute overlapping region between two boxes.
   * @param other Other box.
   * @return Intersection box, or std::nullopt when disjoint.
   */
  std::optional<Box> GetIntersection(const Box& other) const;

  /**
   * @brief Distance from a coordinate to the box.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return Zero for points inside the box; otherwise Euclidean distance.
   */
  double DistanceToPoint(double x, double y) const;

  /**
   * @brief Distance from a point to the box.
   * @param point Point to measure from.
   * @return Zero for points inside the box; otherwise Euclidean distance.
   */
  double DistanceToPoint(const Point& point) const;

  /**
   * @brief Closest point on or in the box to a coordinate.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return Clamped point on the box.
   */
  Point ClosestPointTo(double x, double y) const;

  /**
   * @brief Closest point on or in the box to a point.
   * @param point Point to clamp.
   * @return Clamped point on the box.
   */
  Point ClosestPointTo(const Point& point) const;

  /**
   * @brief Distance between this box and another box.
   * @param other Other box.
   * @return Zero for overlapping boxes; otherwise Euclidean separation.
   */
  double DistanceToBox(const Box& other) const;

  /**
   * @brief Compute diagonal length.
   * @return Length from one corner to the opposite corner.
   */
  double GetDiagonalLength() const;

  /**
   * @brief Compute width divided by height.
   * @return Aspect ratio.
   */
  double GetAspectRatio() const;

  /**
   * @brief Check whether width and height are equal within tolerance.
   * @param tolerance Floating-point tolerance.
   * @return true if the box is square.
   */
  bool IsSquare(double tolerance = 1e-9) const;

  /**
   * @brief Compute the smallest box containing this box and another.
   * @param other Other box.
   * @return Bounding box.
   */
  Box GetBoundingBox(const Box& other) const;

  /**
   * @brief Return the four corners.
   * @return Bottom-left, bottom-right, top-right, top-left corners.
   */
  std::array<Point, 4> GetCorners() const;

  /**
   * @brief Compare boxes with tolerant floating-point equality.
   * @param other Other box.
   * @return true if centers and half dimensions match within tolerance.
   */
  bool operator==(const Box& other) const;

  /**
   * @brief Compare boxes for inequality.
   * @param other Other box.
   * @return true if boxes are not equal.
   */
  bool operator!=(const Box& other) const;

  /**
   * @brief Stream output operator
   * 'friend' allows this function to access private members for printing
   * This lets you use: std::cout << box; to display box information
   */
  friend std::ostream& operator<<(std::ostream& os, const Box& box);

  /**
   * @brief Validate finite coordinates and positive half dimensions.
   * @return true if the box is usable.
   */
  bool IsValid() const;
};

}  // namespace cse498

#endif  // CSE498_BOX_HPP
