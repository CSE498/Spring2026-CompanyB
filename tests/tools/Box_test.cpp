/**
 * @file Box_test.cpp
 * @brief Unit tests for the Box class using Catch2
 * 
 * @author Ebenezer Appiah (Group 13 - Math-Heavy Simulation World)
 */

#include "../source/tools/Box.hpp"
#include "../source/tools/PointClass.hpp"
#include "../source/tools/Circle.hpp"
#include "../third-party/Catch/single_include/catch2/catch.hpp"
#include <iostream>
#include <cmath>
#include <sstream>

using namespace cse498;

// Helper function for floating-point comparison
bool FloatEqual(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

// Constructor tests

TEST_CASE("Default constructor", "[Box][constructor]") {
    Box box;
    REQUIRE(FloatEqual(box.GetCenterX(), 0.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 0.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 1.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 1.0));
    REQUIRE(box.IsValid());
}

TEST_CASE("Parameterized constructor", "[Box][constructor]") {
    Box box(5.0, 10.0, 3.0, 4.0);
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 10.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 3.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 4.0));
    REQUIRE(FloatEqual(box.GetWidth(), 6.0));
    REQUIRE(FloatEqual(box.GetHeight(), 8.0));
}

TEST_CASE("Point constructor", "[Box][constructor]") {
    Point center(5.0, 10.0);
    Box box(center, 3.0, 4.0);
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 10.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 3.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST_CASE("FromCorners factory method", "[Box][constructor]") {
    Point corner1(2.0, 3.0);
    Point corner2(8.0, 11.0);
    Box box = Box::FromCorners(corner1, corner2);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 7.0));
    REQUIRE(FloatEqual(box.GetWidth(), 6.0));
    REQUIRE(FloatEqual(box.GetHeight(), 8.0));
    
    Box box2 = Box::FromCorners(corner2, corner1);
    REQUIRE(box == box2);
}

TEST_CASE("FromBottomLeft factory method", "[Box][constructor]") {
    Point bottom_left(2.0, 3.0);
    Box box = Box::FromBottomLeft(bottom_left, 6.0, 8.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 7.0));
    REQUIRE(FloatEqual(box.GetWidth(), 6.0));
    REQUIRE(FloatEqual(box.GetHeight(), 8.0));
}

// Getter tests

TEST_CASE("Position getters", "[Box][getters]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    
    Point center = box.GetCenter();
    REQUIRE(FloatEqual(center.getX(), 5.0));
    REQUIRE(FloatEqual(center.getY(), 7.0));
    
    Point bottom_left = box.GetBottomLeft();
    REQUIRE(FloatEqual(bottom_left.getX(), 2.0));
    REQUIRE(FloatEqual(bottom_left.getY(), 3.0));
    
    Point top_right = box.GetTopRight();
    REQUIRE(FloatEqual(top_right.getX(), 8.0));
    REQUIRE(FloatEqual(top_right.getY(), 11.0));
}

TEST_CASE("Dimension getters", "[Box][getters]") {
    Box box(0.0, 0.0, 5.0, 7.0);
    
    REQUIRE(FloatEqual(box.GetHalfWidth(), 5.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 7.0));
    REQUIRE(FloatEqual(box.GetWidth(), 10.0));
    REQUIRE(FloatEqual(box.GetHeight(), 14.0));
    REQUIRE(FloatEqual(box.GetArea(), 140.0));
    REQUIRE(FloatEqual(box.GetPerimeter(), 48.0));
}

// Setter tests

TEST_CASE("SetCenter", "[Box][setters]") {
    Box box(0.0, 0.0, 2.0, 3.0);
    box.SetCenter(10.0, 20.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 10.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 20.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 2.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 3.0));
}

TEST_CASE("SetBottomLeft", "[Box][setters]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.SetBottomLeft(0.0, 0.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 3.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 4.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 3.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST_CASE("SetDimensions", "[Box][setters]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.SetDimensions(10.0, 20.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 7.0));
    REQUIRE(FloatEqual(box.GetWidth(), 10.0));
    REQUIRE(FloatEqual(box.GetHeight(), 20.0));
}

// Transformation tests

TEST_CASE("Translate", "[Box][transformations]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.Translate(10.0, -5.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 15.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 2.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 3.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST_CASE("ScaleFromCenter", "[Box][transformations]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.ScaleFromCenter(2.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 7.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 6.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 8.0));
}

TEST_CASE("ScaleFromPoint", "[Box][transformations]") {
    Box box(4.0, 6.0, 2.0, 2.0);
    Point origin(0.0, 0.0);
    box.ScaleFromPoint(2.0, origin);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 8.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 12.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 4.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST_CASE("Expand", "[Box][transformations]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.Expand(1.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 5.0));
    REQUIRE(FloatEqual(box.GetCenterY(), 7.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 4.0));
    REQUIRE(FloatEqual(box.GetHalfHeight(), 5.0));
}

TEST_CASE("Method chaining", "[Box][transformations]") {
    Box box(5.0, 5.0, 2.0, 2.0);
    box.Translate(5.0, 0.0).ScaleFromCenter(2.0).Expand(1.0);
    
    REQUIRE(FloatEqual(box.GetCenterX(), 10.0));
    REQUIRE(FloatEqual(box.GetHalfWidth(), 5.0));
}

// Collision detection tests

TEST_CASE("Contains point", "[Box][collision]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    
    REQUIRE(box.Contains(5.0, 7.0));
    REQUIRE(box.Contains(4.0, 6.0));
    REQUIRE(box.Contains(2.0, 3.0));
    REQUIRE(box.Contains(8.0, 11.0));
    
    REQUIRE_FALSE(box.Contains(1.0, 7.0));
    REQUIRE_FALSE(box.Contains(9.0, 7.0));
    REQUIRE_FALSE(box.Contains(5.0, 2.0));
    REQUIRE_FALSE(box.Contains(5.0, 12.0));
}

TEST_CASE("Overlaps box", "[Box][collision]") {
    Box box1(5.0, 5.0, 2.0, 2.0);
    
    Box box2(6.0, 6.0, 2.0, 2.0);
    REQUIRE(box1.Overlaps(box2));
    REQUIRE(box2.Overlaps(box1));
    
    Box box3(5.0, 5.0, 1.0, 1.0);
    REQUIRE(box1.Overlaps(box3));
    
    Box box4(15.0, 15.0, 2.0, 2.0);
    REQUIRE_FALSE(box1.Overlaps(box4));
}

TEST_CASE("Overlaps circle", "[Box][collision]") {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    Circle circle1(Point(5.0, 5.0), 1.0);
    REQUIRE(box.Overlaps(circle1));
    
    Circle circle2(Point(8.0, 5.0), 2.0);
    REQUIRE(box.Overlaps(circle2));
    
    Circle circle3(Point(15.0, 15.0), 1.0);
    REQUIRE_FALSE(box.Overlaps(circle3));
}

TEST_CASE("CompletelyContains", "[Box][collision]") {
    Box outer(5.0, 5.0, 5.0, 5.0);
    
    Box inner(5.0, 5.0, 2.0, 2.0);
    REQUIRE(outer.CompletelyContains(inner));
    REQUIRE_FALSE(inner.CompletelyContains(outer));
    
    Box partial(8.0, 8.0, 3.0, 3.0);
    REQUIRE_FALSE(outer.CompletelyContains(partial));
}

TEST_CASE("GetIntersection", "[Box][collision]") {
    Box box1(5.0, 5.0, 3.0, 3.0);
    Box box2(7.0, 7.0, 3.0, 3.0);
    
    auto intersection = box1.GetIntersection(box2);
    REQUIRE(intersection.has_value());
    
    REQUIRE(FloatEqual(intersection->GetCenterX(), 6.0));
    REQUIRE(FloatEqual(intersection->GetCenterY(), 6.0));
    REQUIRE(FloatEqual(intersection->GetWidth(), 4.0));
    REQUIRE(FloatEqual(intersection->GetHeight(), 4.0));
    
    Box box3(15.0, 15.0, 2.0, 2.0);
    auto no_intersection = box1.GetIntersection(box3);
    REQUIRE_FALSE(no_intersection.has_value());
}

// Distance calculation tests

TEST_CASE("DistanceToPoint", "[Box][distance]") {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    REQUIRE(FloatEqual(box.DistanceToPoint(5.0, 5.0), 0.0));
    REQUIRE(FloatEqual(box.DistanceToPoint(4.0, 6.0), 0.0));
    REQUIRE(FloatEqual(box.DistanceToPoint(10.0, 5.0), 3.0));
    REQUIRE(FloatEqual(box.DistanceToPoint(5.0, 1.0), 2.0));
}

TEST_CASE("ClosestPointTo", "[Box][distance]") {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    Point p1 = box.ClosestPointTo(5.0, 5.0);
    REQUIRE(FloatEqual(p1.getX(), 5.0));
    REQUIRE(FloatEqual(p1.getY(), 5.0));
    
    Point p2 = box.ClosestPointTo(10.0, 5.0);
    REQUIRE(FloatEqual(p2.getX(), 7.0));
    REQUIRE(FloatEqual(p2.getY(), 5.0));
    
    Point p3 = box.ClosestPointTo(10.0, 10.0);
    REQUIRE(FloatEqual(p3.getX(), 7.0));
    REQUIRE(FloatEqual(p3.getY(), 7.0));
}

TEST_CASE("DistanceToBox", "[Box][distance]") {
    Box box1(0.0, 0.0, 2.0, 2.0);
    
    Box box2(1.0, 1.0, 2.0, 2.0);
    REQUIRE(FloatEqual(box1.DistanceToBox(box2), 0.0));
    
    Box box3(6.0, 0.0, 2.0, 2.0);
    double dist = box1.DistanceToBox(box3);
    REQUIRE(FloatEqual(dist, 2.0));
}

// Geometric query tests

TEST_CASE("GetDiagonalLength", "[Box][geometry]") {
    Box box(0.0, 0.0, 3.0, 4.0);
    double diagonal = box.GetDiagonalLength();
    double expected = std::sqrt(6.0*6.0 + 8.0*8.0);
    REQUIRE(FloatEqual(diagonal, expected));
}

TEST_CASE("GetAspectRatio", "[Box][geometry]") {
    Box box1(0.0, 0.0, 4.0, 2.0);
    REQUIRE(FloatEqual(box1.GetAspectRatio(), 2.0));
    
    Box box2(0.0, 0.0, 2.0, 4.0);
    REQUIRE(FloatEqual(box2.GetAspectRatio(), 0.5));
}

TEST_CASE("IsSquare", "[Box][geometry]") {
    Box square(0.0, 0.0, 3.0, 3.0);
    REQUIRE(square.IsSquare());
    
    Box rectangle(0.0, 0.0, 3.0, 4.0);
    REQUIRE_FALSE(rectangle.IsSquare());
}

TEST_CASE("GetBoundingBox", "[Box][geometry]") {
    Box box1(2.0, 2.0, 1.0, 1.0);
    Box box2(5.0, 5.0, 1.0, 1.0);
    
    Box bounding = box1.GetBoundingBox(box2);
    
    REQUIRE(FloatEqual(bounding.GetBottomLeft().getX(), 1.0));
    REQUIRE(FloatEqual(bounding.GetBottomLeft().getY(), 1.0));
    REQUIRE(FloatEqual(bounding.GetTopRight().getX(), 6.0));
    REQUIRE(FloatEqual(bounding.GetTopRight().getY(), 6.0));
}

// Operator tests

TEST_CASE("Equality operator", "[Box][operators]") {
    Box box1(5.0, 7.0, 3.0, 4.0);
    Box box2(5.0, 7.0, 3.0, 4.0);
    Box box3(5.0, 7.0, 3.0, 4.001);
    
    REQUIRE(box1 == box2);
    REQUIRE_FALSE(box1 == box3);
    REQUIRE(box1 != box3);
    REQUIRE_FALSE(box1 != box2);
}

TEST_CASE("Output stream operator", "[Box][operators]") {
    Box box(5.0, 7.0, 3.0, 4.0);
    std::ostringstream oss;
    oss << box;
    std::string output = oss.str();
    
    REQUIRE(output.find("Box") != std::string::npos);
    REQUIRE(output.find("5") != std::string::npos);
}

// Edge case tests

TEST_CASE("Very small box", "[Box][edge_cases]") {
    Box tiny(0.0, 0.0, 1e-10, 1e-10);
    REQUIRE(tiny.IsValid());
}

TEST_CASE("Very large box", "[Box][edge_cases]") {
    Box huge(0.0, 0.0, 1e10, 1e10);
    REQUIRE(huge.IsValid());
}

TEST_CASE("Negative coordinates", "[Box][edge_cases]") {
    Box box(-10.0, -20.0, 5.0, 7.0);
    REQUIRE(FloatEqual(box.GetCenterX(), -10.0));
    REQUIRE(FloatEqual(box.GetCenterY(), -20.0));
    REQUIRE(box.IsValid());
}

// Validation tests

TEST_CASE("IsValid", "[Box][validation]") {
    Box valid(5.0, 7.0, 3.0, 4.0);
    REQUIRE(valid.IsValid());
}
