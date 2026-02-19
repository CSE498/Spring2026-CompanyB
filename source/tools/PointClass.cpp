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
    //return (std::abs(lhs.getX() - rhs.getX()) <= EPSILON && 
        //std::abs(lhs.getY() - rhs.getY()) <= EPSILON );

   //return (lhs.getX() == rhs.getX() && lhs.getY() == rhs.getY()); 
    return ( tol_equal(lhs.getX(), rhs.getX()) && tol_equal(lhs.getY(), rhs.getY()) );
}

// equality (==) between a point and a scalar
bool operator==(const Point& lhs, double rhs){
    return false;
}

//  equality (==) between a scalar and a point
bool operator==(double lhs, const Point& rhs){
    return false;
}


// The unequal "operators"
bool operator!=(const Point& lhs, const Point& rhs){
   if ( lhs.getX() != rhs.getX() || lhs.getY() != rhs.getY() )
        return true;
    return false;
}
bool operator!=(const Point& lhs, double rhs){
    //std::cout << "You cannot compare a scalar and a Point" << std::endl;
    return false;
}
bool operator!=(double lhs, const Point& rhs){
    //std::cout << "You cannot compare a scalar and a Point" << std::endl;
    return false;
}


double dot(const Point& A, const Point& B){
    return (( A.getX() * B.getX()) + (A.getY() * B.getY() ));
}

} // namespace cse498
