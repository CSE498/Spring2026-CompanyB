// PointClass.h
#pragma once 

#include <iostream>
#include <cmath> // for std::abs, std::sqrt, std::cos, std::sin
#include <algorithm> // for std::max
#include <numbers> // for std::numbers::pi


namespace cse498 {

constexpr double EPSILON = 1e-9;
constexpr double RELATIVE_TOLERANCE= 1e-12;
inline bool tol_equal(double a, double b){
    return std::abs(a - b) <= EPSILON + RELATIVE_TOLERANCE * std::max(std::abs(a), std::abs(b));
}

class Point{

private:
    double x{}, y{};
    
public:
    // default constructor
    Point() : x(0.0), y(0.0) {} 


    // disable assignment constructor 
    //Point& operator=(const Point&) = delete;

    // parameterized constructor 
    Point(double X, double Y) : x(X), y(Y) {} 

    // getters to get value of x and y
    double getX() const{ return x;}
    double getY() const{ return y;}


    // setters to set value of x and y 
    Point& setX(double X=0.0) { this->x = X; return *this;}
    Point& setY(double Y=0.0) { this->y = Y; return *this; }

    // operators
    // addition (+=)
    Point& operator+=(const Point& other){
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    // subtraction (-=)
    Point& operator-=(const Point& other){
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    // product (*)
    //Point& operator*(const Point& other){
    //    this->x *= other.x;
    //    this->y *= other.y;
    //    return *this;
    //}

    // equality (==)
    //bool operator==(const Point& other){
    //    if (this->y == other.y && this->x == other.x)
    //        return true;
    //    else
    //        return false;
    //}
    // equality (!=)

    //bool operator!=(const Point& other){
    //    if (this->y != other.y || this->x != other.x)
    //        return true;
    //    else
    //        return false;
    //}

    //3. Geometry: dot and cross product, magnitude, normalize(return unit vector),
    double dot(const Point& A){
        return (( this->x* A.getX() ) + ( this->y * A.getY() ));
    }


    double magnitude() const{
        double X_squared = this->x *this->x;
        double Y_squared = this->y * this->y;
        double square_sum = (X_squared + Y_squared);
        double square_root = std::sqrt(square_sum);
        return square_root;
        
    }
    //4. Transforms: rotate, scale
    // scale method -- I might delete this in the future 
    // since I already have operator*
    Point& scale(double scalar){
        x = this->x * scalar;
        y = this->y * scalar;
        return *this;
    }

    // normalize
    Point& normalize(){
        double mag = this->magnitude(); 
        if (mag != 0){
            x = this->x / mag;
            y = this->y / mag;
        }

        return *this;
    }
    
    // rotate 
    Point& rotate(double deg, Point pivot = {0,0},
                  bool counter_clockwise = true){ 
        //return *this; // I will implement this later.
        if (tol_equal(deg, 0)) return *this;
        if (!counter_clockwise) deg = -deg;

        double rad = deg * (std::numbers::pi / 180.0); 

        double tempx = x - pivot.x; 
        double tempy = y - pivot.y; 

        double c = std::cos(rad);
        double s = std::sin(rad);

        x = (( tempx*c ) - ( tempy*s )) + pivot.x;
        y = (( tempx*s ) + ( tempy*c )) + pivot.y;

        return *this;
    }


    // 2D cross product -- useful for finding area of the parallelogram
    // area of triangle, possibly torque
    // 2D cross product yields a fake "k" in ijk system 
    // my method will return a scalar, not a point vector 
    double cross_product(const Point& other) const {
        return (this->x * other.y) - (this->y * other.x);
    }


};

// The addition "operators"
Point operator+(const Point& lhs, const Point& rhs);
Point operator+(const Point& lhs, double rhs);
Point operator+(double lhs, const Point& rhs);

// The subtraction "operators"
Point operator-(const Point& lhs, const Point& rhs);
Point operator-(const Point& lhs, double rhs);
Point operator-(double lhs, const Point& rhs);

// The product "operators"
Point operator*(const Point& lhs, const Point& rhs);
Point operator*(const Point& lhs, double rhs);
Point operator*(double lhs, const Point& rhs);

// The equality "operators"
bool operator==(const Point& lhs, const Point& rhs);
bool operator==(const Point& lhs, double rhs);
bool operator==(double lhs, const Point& rhs);


// The unequal "operators"
bool operator!=(const Point& lhs, const Point& rhs);
bool operator!=(const Point& lhs, double rhs);
bool operator!=(double lhs, const Point& rhs);

// dot product
double dot(const Point& A, const Point& B);

} // namespace cse498
