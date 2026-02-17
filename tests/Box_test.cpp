/**
 * @file Box_test.cpp
 * @brief Unit tests for the Box class
 * 
 * @author Ebenezer Appiah (Group 13 - Math-Heavy Simulation World)
 */

#include "Box.hpp"
#include "Point.hpp"
#include "Circle.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <sstream>

namespace cse498 {
namespace test {

bool FloatEqual(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void name(); \
    struct name##_runner { \
        name##_runner() { \
            std::cout << "Running " << #name << "..." << std::endl; \
            try { name(); tests_passed++; std::cout << "  PASSED" << std::endl; } \
            catch (const std::exception& e) { \
                tests_failed++; \
                std::cout << "  FAILED: " << e.what() << std::endl; \
            } \
        } \
    } name##_instance; \
    void name()

// Constructor tests

TEST(TestDefaultConstructor) {
    Box box;
    assert(FloatEqual(box.GetCenterX(), 0.0));
    assert(FloatEqual(box.GetCenterY(), 0.0));
    assert(FloatEqual(box.GetHalfWidth(), 1.0));
    assert(FloatEqual(box.GetHalfHeight(), 1.0));
    assert(box.IsValid());
}

TEST(TestParameterizedConstructor) {
    Box box(5.0, 10.0, 3.0, 4.0);
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 10.0));
    assert(FloatEqual(box.GetHalfWidth(), 3.0));
    assert(FloatEqual(box.GetHalfHeight(), 4.0));
    assert(FloatEqual(box.GetWidth(), 6.0));
    assert(FloatEqual(box.GetHeight(), 8.0));
}

TEST(TestPointConstructor) {
    Point center(5.0, 10.0);
    Box box(center, 3.0, 4.0);
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 10.0));
    assert(FloatEqual(box.GetHalfWidth(), 3.0));
    assert(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST(TestFromCorners) {
    Point corner1(2.0, 3.0);
    Point corner2(8.0, 11.0);
    Box box = Box::FromCorners(corner1, corner2);
    
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 7.0));
    assert(FloatEqual(box.GetWidth(), 6.0));
    assert(FloatEqual(box.GetHeight(), 8.0));
    
    Box box2 = Box::FromCorners(corner2, corner1);
    assert(box == box2);
}

TEST(TestFromBottomLeft) {
    Point bottom_left(2.0, 3.0);
    Box box = Box::FromBottomLeft(bottom_left, 6.0, 8.0);
    
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 7.0));
    assert(FloatEqual(box.GetWidth(), 6.0));
    assert(FloatEqual(box.GetHeight(), 8.0));
}

// Getter tests

TEST(TestPositionGetters) {
    Box box(5.0, 7.0, 3.0, 4.0);
    
    Point center = box.GetCenter();
    assert(FloatEqual(center.GetX(), 5.0));
    assert(FloatEqual(center.GetY(), 7.0));
    
    Point bottom_left = box.GetBottomLeft();
    assert(FloatEqual(bottom_left.GetX(), 2.0));
    assert(FloatEqual(bottom_left.GetY(), 3.0));
    
    Point top_right = box.GetTopRight();
    assert(FloatEqual(top_right.GetX(), 8.0));
    assert(FloatEqual(top_right.GetY(), 11.0));
}

TEST(TestDimensionGetters) {
    Box box(0.0, 0.0, 5.0, 7.0);
    
    assert(FloatEqual(box.GetHalfWidth(), 5.0));
    assert(FloatEqual(box.GetHalfHeight(), 7.0));
    assert(FloatEqual(box.GetWidth(), 10.0));
    assert(FloatEqual(box.GetHeight(), 14.0));
    assert(FloatEqual(box.GetArea(), 140.0));
    assert(FloatEqual(box.GetPerimeter(), 48.0));
}

// Setter tests

TEST(TestSetCenter) {
    Box box(0.0, 0.0, 2.0, 3.0);
    box.SetCenter(10.0, 20.0);
    
    assert(FloatEqual(box.GetCenterX(), 10.0));
    assert(FloatEqual(box.GetCenterY(), 20.0));
    assert(FloatEqual(box.GetHalfWidth(), 2.0));
    assert(FloatEqual(box.GetHalfHeight(), 3.0));
}

TEST(TestSetBottomLeft) {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.SetBottomLeft(0.0, 0.0);
    
    assert(FloatEqual(box.GetCenterX(), 3.0));
    assert(FloatEqual(box.GetCenterY(), 4.0));
    assert(FloatEqual(box.GetHalfWidth(), 3.0));
    assert(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST(TestSetDimensions) {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.SetDimensions(10.0, 20.0);
    
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 7.0));
    assert(FloatEqual(box.GetWidth(), 10.0));
    assert(FloatEqual(box.GetHeight(), 20.0));
}

// Transformation tests

TEST(TestTranslate) {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.Translate(10.0, -5.0);
    
    assert(FloatEqual(box.GetCenterX(), 15.0));
    assert(FloatEqual(box.GetCenterY(), 2.0));
    assert(FloatEqual(box.GetHalfWidth(), 3.0));
    assert(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST(TestScaleFromCenter) {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.ScaleFromCenter(2.0);
    
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 7.0));
    assert(FloatEqual(box.GetHalfWidth(), 6.0));
    assert(FloatEqual(box.GetHalfHeight(), 8.0));
}

TEST(TestScaleFromPoint) {
    Box box(4.0, 6.0, 2.0, 2.0);
    Point origin(0.0, 0.0);
    box.ScaleFromPoint(2.0, origin);
    
    assert(FloatEqual(box.GetCenterX(), 8.0));
    assert(FloatEqual(box.GetCenterY(), 12.0));
    assert(FloatEqual(box.GetHalfWidth(), 4.0));
    assert(FloatEqual(box.GetHalfHeight(), 4.0));
}

TEST(TestExpand) {
    Box box(5.0, 7.0, 3.0, 4.0);
    box.Expand(1.0);
    
    assert(FloatEqual(box.GetCenterX(), 5.0));
    assert(FloatEqual(box.GetCenterY(), 7.0));
    assert(FloatEqual(box.GetHalfWidth(), 4.0));
    assert(FloatEqual(box.GetHalfHeight(), 5.0));
}

TEST(TestMethodChaining) {
    Box box(5.0, 5.0, 2.0, 2.0);
    box.Translate(5.0, 0.0).ScaleFromCenter(2.0).Expand(1.0);
    
    assert(FloatEqual(box.GetCenterX(), 10.0));
    assert(FloatEqual(box.GetHalfWidth(), 5.0));
}

// Collision detection tests

TEST(TestContainsPoint) {
    Box box(5.0, 7.0, 3.0, 4.0);
    
    assert(box.Contains(5.0, 7.0));
    assert(box.Contains(4.0, 6.0));
    assert(box.Contains(2.0, 3.0));
    assert(box.Contains(8.0, 11.0));
    
    assert(!box.Contains(1.0, 7.0));
    assert(!box.Contains(9.0, 7.0));
    assert(!box.Contains(5.0, 2.0));
    assert(!box.Contains(5.0, 12.0));
}

TEST(TestOverlapsBox) {
    Box box1(5.0, 5.0, 2.0, 2.0);
    
    Box box2(6.0, 6.0, 2.0, 2.0);
    assert(box1.Overlaps(box2));
    assert(box2.Overlaps(box1));
    
    Box box3(5.0, 5.0, 1.0, 1.0);
    assert(box1.Overlaps(box3));
    
    Box box4(15.0, 15.0, 2.0, 2.0);
    assert(!box1.Overlaps(box4));
}

TEST(TestOverlapsCircle) {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    Circle circle1(5.0, 5.0, 1.0);
    assert(box.Overlaps(circle1));
    
    Circle circle2(8.0, 5.0, 2.0);
    assert(box.Overlaps(circle2));
    
    Circle circle3(15.0, 15.0, 1.0);
    assert(!box.Overlaps(circle3));
}

TEST(TestCompletelyContains) {
    Box outer(5.0, 5.0, 5.0, 5.0);
    
    Box inner(5.0, 5.0, 2.0, 2.0);
    assert(outer.CompletelyContains(inner));
    assert(!inner.CompletelyContains(outer));
    
    Box partial(8.0, 8.0, 3.0, 3.0);
    assert(!outer.CompletelyContains(partial));
}

TEST(TestGetIntersection) {
    Box box1(5.0, 5.0, 3.0, 3.0);
    Box box2(7.0, 7.0, 3.0, 3.0);
    
    auto intersection = box1.GetIntersection(box2);
    assert(intersection.has_value());
    
    assert(FloatEqual(intersection->GetCenterX(), 6.0));
    assert(FloatEqual(intersection->GetCenterY(), 6.0));
    assert(FloatEqual(intersection->GetWidth(), 4.0));
    assert(FloatEqual(intersection->GetHeight(), 4.0));
    
    Box box3(15.0, 15.0, 2.0, 2.0);
    auto no_intersection = box1.GetIntersection(box3);
    assert(!no_intersection.has_value());
}

// Distance calculation tests

TEST(TestDistanceToPoint) {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    assert(FloatEqual(box.DistanceToPoint(5.0, 5.0), 0.0));
    assert(FloatEqual(box.DistanceToPoint(4.0, 6.0), 0.0));
    assert(FloatEqual(box.DistanceToPoint(10.0, 5.0), 3.0));
    assert(FloatEqual(box.DistanceToPoint(5.0, 1.0), 2.0));
}

TEST(TestClosestPointTo) {
    Box box(5.0, 5.0, 2.0, 2.0);
    
    Point p1 = box.ClosestPointTo(5.0, 5.0);
    assert(FloatEqual(p1.GetX(), 5.0) && FloatEqual(p1.GetY(), 5.0));
    
    Point p2 = box.ClosestPointTo(10.0, 5.0);
    assert(FloatEqual(p2.GetX(), 7.0) && FloatEqual(p2.GetY(), 5.0));
    
    Point p3 = box.ClosestPointTo(10.0, 10.0);
    assert(FloatEqual(p3.GetX(), 7.0) && FloatEqual(p3.GetY(), 7.0));
}

TEST(TestDistanceToBox) {
    Box box1(0.0, 0.0, 2.0, 2.0);
    
    Box box2(1.0, 1.0, 2.0, 2.0);
    assert(FloatEqual(box1.DistanceToBox(box2), 0.0));
    
    Box box3(6.0, 0.0, 2.0, 2.0);
    double dist = box1.DistanceToBox(box3);
    assert(FloatEqual(dist, 2.0));
}

// Geometric query tests

TEST(TestGetDiagonalLength) {
    Box box(0.0, 0.0, 3.0, 4.0);
    double diagonal = box.GetDiagonalLength();
    double expected = std::sqrt(6.0*6.0 + 8.0*8.0);
    assert(FloatEqual(diagonal, expected));
}

TEST(TestGetAspectRatio) {
    Box box1(0.0, 0.0, 4.0, 2.0);
    assert(FloatEqual(box1.GetAspectRatio(), 2.0));
    
    Box box2(0.0, 0.0, 2.0, 4.0);
    assert(FloatEqual(box2.GetAspectRatio(), 0.5));
}

TEST(TestIsSquare) {
    Box square(0.0, 0.0, 3.0, 3.0);
    assert(square.IsSquare());
    
    Box rectangle(0.0, 0.0, 3.0, 4.0);
    assert(!rectangle.IsSquare());
}

TEST(TestGetBoundingBox) {
    Box box1(2.0, 2.0, 1.0, 1.0);
    Box box2(5.0, 5.0, 1.0, 1.0);
    
    Box bounding = box1.GetBoundingBox(box2);
    
    assert(FloatEqual(bounding.GetBottomLeft().GetX(), 1.0));
    assert(FloatEqual(bounding.GetBottomLeft().GetY(), 1.0));
    assert(FloatEqual(bounding.GetTopRight().GetX(), 6.0));
    assert(FloatEqual(bounding.GetTopRight().GetY(), 6.0));
}

// Operator tests

TEST(TestEqualityOperator) {
    Box box1(5.0, 7.0, 3.0, 4.0);
    Box box2(5.0, 7.0, 3.0, 4.0);
    Box box3(5.0, 7.0, 3.0, 4.001);
    
    assert(box1 == box2);
    assert(!(box1 == box3));
    assert(box1 != box3);
    assert(!(box1 != box2));
}

TEST(TestOutputStreamOperator) {
    Box box(5.0, 7.0, 3.0, 4.0);
    std::ostringstream oss;
    oss << box;
    std::string output = oss.str();
    
    assert(output.find("Box") != std::string::npos);
    assert(output.find("5") != std::string::npos);
}

// Edge case tests

TEST(TestVerySmallBox) {
    Box tiny(0.0, 0.0, 1e-10, 1e-10);
    assert(tiny.IsValid());
}

TEST(TestVeryLargeBox) {
    Box huge(0.0, 0.0, 1e10, 1e10);
    assert(huge.IsValid());
}

TEST(TestNegativeCoordinates) {
    Box box(-10.0, -20.0, 5.0, 7.0);
    assert(FloatEqual(box.GetCenterX(), -10.0));
    assert(FloatEqual(box.GetCenterY(), -20.0));
    assert(box.IsValid());
}

// Validation tests

TEST(TestIsValid) {
    Box valid(5.0, 7.0, 3.0, 4.0);
    assert(valid.IsValid());
}

} // namespace test
} // namespace cse498

int main() {
    using namespace cse498::test;
    
    std::cout << "\n=== Box Class Unit Tests ===\n" << std::endl;
    
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Tests passed: " << tests_passed << std::endl;
    std::cout << "Tests failed: " << tests_failed << std::endl;
    std::cout << "Total tests: " << (tests_passed + tests_failed) << std::endl;
    
    if (tests_failed == 0) {
        std::cout << "\n✓ All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\n✗ Some tests failed!" << std::endl;
        return 1;
    }
}
