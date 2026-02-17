/**
 * @file Box.hpp
 * @brief Axis-Aligned Bounding Box (AABB) class for 2D geometric operations
 * 
 * @author Ebenezer Appiah (Group 13 - Math-Heavy Simulation World)
 */

#ifndef CSE498_BOX_HPP
#define CSE498_BOX_HPP

#include "Point.hpp"
#include "Circle.hpp"
#include <cmath>
#include <optional>
#include <iostream>
#include <cassert>
#include <array>
#include <algorithm>

namespace cse498 {

/**
 * @class Box
 * @brief Represents an axis-aligned 2D rectangular box
 */
class Box {
private:
    double center_x_;
    double center_y_;
    double half_width_;
    double half_height_;

public:
    // Constructors
    Box();
    Box(double center_x, double center_y, double half_width, double half_height);
    Box(const Point& center, double half_width, double half_height);
    static Box FromCorners(const Point& corner1, const Point& corner2);
    static Box FromBottomLeft(const Point& bottom_left, double width, double height);
    
    // Position getters
    Point GetCenter() const;
    double GetCenterX() const { return center_x_; }
    double GetCenterY() const { return center_y_; }
    Point GetBottomLeft() const;
    Point GetTopRight() const;
    Point GetBottomRight() const;
    Point GetTopLeft() const;
    
    // Dimension getters
    double GetHalfWidth() const { return half_width_; }
    double GetHalfHeight() const { return half_height_; }
    double GetWidth() const { return 2.0 * half_width_; }
    double GetHeight() const { return 2.0 * half_height_; }
    double GetArea() const { return GetWidth() * GetHeight(); }
    double GetPerimeter() const { return 2.0 * (GetWidth() + GetHeight()); }
    
    // Position setters
    void SetCenter(double x, double y);
    void SetCenter(const Point& center);
    void SetBottomLeft(double x, double y);
    void SetBottomLeft(const Point& bottom_left);
    
    // Dimension setters
    void SetHalfDimensions(double half_width, double half_height);
    void SetDimensions(double width, double height);
    
    // Transformations (support method chaining)
    Box& Translate(double dx, double dy);
    Box& Translate(const Point& displacement);
    Box& ScaleFromCenter(double scale_factor);
    Box& ScaleFromCenter(double scale_x, double scale_y);
    Box& ScaleFromPoint(double scale_factor, double origin_x, double origin_y);
    Box& ScaleFromPoint(double scale_factor, const Point& origin);
    Box& Expand(double amount);
    Box& Expand(double horizontal_amount, double vertical_amount);
    
    // Collision detection
    bool Contains(double x, double y) const;
    bool Contains(const Point& point) const;
    bool Overlaps(const Box& other) const;
    bool Overlaps(const Circle& circle) const;
    bool CompletelyContains(const Box& other) const;
    std::optional<Box> GetIntersection(const Box& other) const;
    
    // Distance calculations
    double DistanceToPoint(double x, double y) const;
    double DistanceToPoint(const Point& point) const;
    Point ClosestPointTo(double x, double y) const;
    Point ClosestPointTo(const Point& point) const;
    double DistanceToBox(const Box& other) const;
    
    // Geometric queries
    double GetDiagonalLength() const;
    double GetAspectRatio() const;
    bool IsSquare(double tolerance = 1e-9) const;
    Box GetBoundingBox(const Box& other) const;
    std::array<Point, 4> GetCorners() const;
    
    // Operators
    bool operator==(const Box& other) const;
    bool operator!=(const Box& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Box& box);
    
    // Validation
    bool IsValid() const;
};

} // namespace cse498

#endif // CSE498_BOX_HPP
