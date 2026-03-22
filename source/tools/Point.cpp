#include "Point.hpp"

namespace cse498 {
/* Additions
 * addition (+) between two points
 * example: Point origin(0,0) + Point A(1,1)
 * that equals Point result(0+1, 0+1) = (1,1)
 */
Point operator+(const Point& lhs, const Point& rhs) {
  return Point(lhs.getX() + rhs.getX(), lhs.getY() + rhs.getY());
}

/* Additions
 * addtion (+) between a point and a scalar
 * example: Point A(2,0) + 4
 * that equals Point res(2+4, 0+4) = (6,4)
 */
Point operator+(const Point& lhs, double rhs) {
  return Point(lhs.getX() + rhs, lhs.getY() + rhs);
}

/* Additions
 * addtion (+) between a point and a scalar
 * example: 4 + Point A(2,0)
 * that equals Point res(2+4, 0+4) = (6,4)
 */
Point operator+(double lhs, const Point& rhs) {
  return Point(lhs + rhs.getX(), lhs + rhs.getY());
}

/* Subtractions
 * subtraction (-) between two points
 * example: Point A(5,3) - Point B(1,2) = Point(4,1)
 */
Point operator-(const Point& lhs, const Point& rhs) {
  return Point(lhs.getX() - rhs.getX(), lhs.getY() - rhs.getY());
}

/* Subtractions
 * subraction (-) between a point and a scalar
 * example: Point A(5,3) - 2 = Point(3,1)
 */
Point operator-(const Point& lhs, double rhs) {
  return Point(lhs.getX() - rhs, lhs.getY() - rhs);
}

/* Subtractions
 * subraction (-) between a scalar and a point
 * example: Point 2 - A(5,3) = Point(-3,-1)
 */
Point operator-(double lhs, const Point& rhs) {
  return Point(lhs - rhs.getX(), lhs - rhs.getY());
}

/* Products
 * element-wise product (*) between two points
 * example: Point A(2,3) * Point B(4,5) = Point(8,15)
 */
Point operator*(const Point& lhs, const Point& rhs) {
  return Point(lhs.getX() * rhs.getX(), lhs.getY() * rhs.getY());
}

/* Products
 * element-wise product (*) between a point and a scalar
 * example: Point A(2,3) * 4 = Point(8,12)
 */
Point operator*(const Point& lhs, double rhs) {
  Point result = lhs;
  return result.scale(rhs);
}

/* Products
 * element-wise product (*) between a scalr and a point
 * example: Point 4 * A(2,3) = Point(8,12)
 */
Point operator*(double lhs, const Point& rhs) {
  Point result = rhs;
  return result.scale(lhs);
}

/* Equality
 * tolerance-based equality (==) between two points
 * uses tol_equal to handle floating-point imprecision
 * example: Point(1.0, 2.0) == Point(1.0, 2.0) → true
 */
bool operator==(const Point& lhs, const Point& rhs) {
  return (tol_equal(lhs.getX(), rhs.getX()) &&
          tol_equal(lhs.getY(), rhs.getY()));
}

/* Equality
 * tolerance-based equality (==) between point and a scalar
 * uses tol_equal to handle floating-point imprecision
 * example: Point(1.0, 1.0) == 1.0 → true
 */
bool operator==(const Point& lhs, double rhs) {
  return tol_equal(lhs.getX(), rhs) && tol_equal(lhs.getY(), rhs);
}

/* Equality
 * tolerance-based equality (==) between a scalar and a point
 * uses tol_equal to handle floating-point imprecision
 * example: 1.0 == Point(1.0, 1.0) → true
 */
bool operator==(double lhs, const Point& rhs) {
  return tol_equal(lhs, rhs.getX()) && tol_equal(lhs, rhs.getY());
}

// The unequal "operators"
bool operator!=(const Point& lhs, const Point& rhs) { return !(lhs == rhs); }

bool operator!=(const Point& lhs, double rhs) { return !(lhs == rhs); }

bool operator!=(double lhs, const Point& rhs) { return !(lhs == rhs); }

double dot(const Point& A, const Point& B) { return A.dot(B); }

}  // namespace cse498
