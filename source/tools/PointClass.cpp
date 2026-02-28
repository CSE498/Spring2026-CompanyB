#include "PointClass.hpp"

namespace cse498 {
/* Additions */
// addition (+) between two points
Point operator+(const Point& lhs, const Point& rhs){
    return Point(lhs.getX() + rhs.getX(),lhs.getY() + rhs.getY());
}

// addtion (+) between a point and a scalar
Point operator+(const Point& lhs, double rhs){
    return Point(lhs.getX() + rhs, lhs.getY() + rhs);
}

// addition (+) between a scalar and a point
Point operator+(double lhs, const Point& rhs){
    return Point( lhs + rhs.getX(), lhs + rhs.getY() );
}



/* Subtractions */
// subraction (-) between two points
Point operator-(const Point& lhs, const Point& rhs){
    return Point(lhs.getX() - rhs.getX(),lhs.getY() - rhs.getY());
}

// subraction (-) between a point and a scalar
Point operator-(const Point& lhs, double rhs){
    return Point(lhs.getX() - rhs, lhs.getY() - rhs);
}

//  subtraction (-) between a scalar and a point
Point operator-(double lhs, const Point& rhs){
    return Point( lhs - rhs.getX(), lhs - rhs.getY() );
}


/* Products */
// product (*) between two points
Point operator*(const Point& lhs, const Point& rhs){
    return Point(lhs.getX() * rhs.getX(),lhs.getY() * rhs.getY());
}

// product (*) between a point and a scalar
Point operator*(const Point& lhs, double rhs){
    return Point(lhs.getX() * rhs, lhs.getY() * rhs);
}

//  product (*) between a scalar and a point
Point operator*(double lhs, const Point& rhs){
    return Point( lhs * rhs.getX(), lhs * rhs.getY() );
}

/* Equality */
// equality (==) between two points
bool operator==(const Point& lhs, const Point& rhs){
    return ( tol_equal(lhs.getX(), rhs.getX()) && tol_equal(lhs.getY(), rhs.getY()) );
}

// equality (==) between a point and a scalar
bool operator==(const Point& lhs [[maybe_unused]], double rhs [[maybe_unused]] ){
    return false;
}

//  equality (==) between a scalar and a point
bool operator==(double lhs [[maybe_unused]], const Point& rhs [[maybe_unused]]){
    return false;
}


// The unequal "operators"
bool operator!=(const Point& lhs, const Point& rhs){
   if ( lhs.getX() != rhs.getX() || lhs.getY() != rhs.getY() )
        return true;
    return false;
}


bool operator!=(const Point& lhs [[maybe_unused]], double rhs [[maybe_unused]]){
    return true;
}

bool operator!=( double lhs [[maybe_unused]], const Point& rhs [[maybe_unused]] ) {
    return true;
}

double dot(const Point& A, const Point& B){
    return (( A.getX() * B.getX()) + (A.getY() * B.getY() ));
}

} // namespace cse498
