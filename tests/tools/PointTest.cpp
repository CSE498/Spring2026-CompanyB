#include "PointClass.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace cse498;


TEST_CASE("Testing Constructors, setters/getters", "[Point][constructor]"){
    Point A;
    CHECK(A.getX() == 0);
    CHECK(A.getY() == 0);

    Point B(11.3, 15.7);
    CHECK(B.getX() == 11.3);
    CHECK(B.getY() == 15.7);

    A = Point(8, 51.6); 
    CHECK(A.getX() == 8);
    CHECK(A.getY() == 51.6);

    B.setY(7).setX();
    CHECK(B.getY() == 7);
    CHECK(B.getX() == 0);

    A.setX(21);
    CHECK(A.getX() == 21);

    Point* C = new Point(A.getX(), A.getY());
    CHECK(A == (*C));
    delete C; 

};

TEST_CASE("Testing addition operator", "[Point][operator+]"){
    Point A(12, 13);
    Point B(1.2, 7.8);
    Point C;

    C = A + B;
    CHECK(C.getX() == 13.2);
    CHECK(C.getY() == 20.8);
    
    CHECK((A+B).getX() == 13.2);
    double scalar1 = 2.7;
    Point D(14.7, 15.7);
    CHECK((A+scalar1) == D);

  	Point Z(12, 13);
    double s = 2.7;

    Point left = s + Z;
    Point right = Z + s;

    CHECK(left.getX() == right.getX());
    CHECK(left.getY() == right.getY());

    // Expected numeric result
    CHECK(left.getX() == 14.7);
    CHECK(left.getY() == 15.7);

	Point M(5.5, -3.0);
    Point zero(0, 0);

    Point N = M + zero;
    CHECK(N == M);

    Point K = M + 0.0;
    CHECK(K == M);

    Point R = 0.0 + M;
    CHECK(R == M);
};

TEST_CASE("Testing subtraction operator", "[Point][operator-]"){
	Point A(12, 13);
    Point B(1.2, 7.8);

    Point C = A - B;
    CHECK(C.getX() == 10.8);
    CHECK(C.getY() == 5.2);

    // A - A = (0,0)
    Point Z = A - A;
    CHECK(Z.getX() == 0);
    CHECK(Z.getY() == 0);

    double s = 2.7;

    Point M = A - s;
    CHECK(M.getX() == 9.3);
    CHECK(M.getY() == 10.3);

    Point N = s - A;
    CHECK(N.getX() == (2.7 - 12));  // -9.3
    CHECK(N.getY() == (2.7 - 13));  // -10.3
 
	Point Q(12, 13);
    CHECK( (Q - s) == (Q + (-s)) );
};



TEST_CASE("Testing multiplication operator", "[Point][operator*]"){
	Point A(2, 3);
    Point B(4, 5);

    Point C = A * B;
    CHECK(C.getX() == 8);
    CHECK(C.getY() == 15);

    Point M(2, -3);
    double s = 2.76;

    Point D = M * s;
    CHECK(D.getX() == 5.52);
    CHECK(D.getY() == -8.28);

    Point F = s * M;
    CHECK(F == D);

    Point I(5.5, -3.0);

    Point I1 = I * 1.0;
    Point I2 = 1.0 * I;
    CHECK(I1 == I);
    CHECK(I2 == I);

    Point Z1 = I * 0.000;
    CHECK(Z1.getX() == 0.0);
    CHECK(Z1.getY() == 0.0);
};


TEST_CASE("Testing equality operator", "[Point][equal==]"){
    Point A(12, 13);
    Point B(12, 13);
    Point C(12, 14);
    const Point V(1.2, 7.8);

    CHECK(A == A);
    CHECK(A == B);
    CHECK(B == A);

    CHECK(A != C);
    CHECK(C != A);

	CHECK(V == V);

    double s = 12;

    CHECK_FALSE(A != s);
    CHECK_FALSE(s == A);
};

TEST_CASE("Testing dot method", "[Point][dot]"){
	Point A(-1.3, 5);
	Point B(15.1, 7.12);
	Point E(15.1, 7.12);

	CHECK(A.dot(A) == 26.69);
	CHECK(B.dot(B) == E.dot(E));

	Point C(3, 7);
	Point D(2, 5);
	CHECK(C.dot(D) == 41); 	
	CHECK(D.dot(C) == 41);

    Point Z(0, 0);

    CHECK(A.dot(Z) == 0);
    CHECK(Z.dot(A) == 0);

    Point M(1, 2);
    Point N(3, 4);
    Point O(5, 6);

    CHECK(M.dot(N + O) == (M.dot(N) + M.dot(O)));
};


TEST_CASE("Testing magnitude method", "[Point][magnitude]"){
    Point Z(0, 0);
    CHECK(Z.magnitude() == 0.0);

    Point A(3, 4);
    CHECK(A.magnitude() == 5.0);

    Point B(-3, -4);
    CHECK(B.magnitude() == 5.0);

///
    Point X(7, 0);
    CHECK(X.magnitude() == 7.0);

    Point Y(0, -2);
    CHECK(Y.magnitude() == 2.0);
///
    Point M(0.5, 0.0);
    CHECK(M.magnitude() == 0.5);

    Point Q(0.5, 0.25);
    double expected = std::sqrt(5.0) / 4.0;

    CHECK(Q.magnitude() ==  expected);

	CHECK(A.magnitude() * A.magnitude() == A.dot(A) );
	CHECK(tol_equal(Q.magnitude() * Q.magnitude() , Q.dot(Q) ));
};


TEST_CASE("Testing scale method", "[Point][scale]"){
	Point A(2, 3);
    A.scale(4);

    CHECK(A.getX() == 8);
    CHECK(A.getY() == 12);	

	Point B(2.5, -3.0);
    Point original = B;

    B.scale(1.0);
    CHECK(B == original);
///
    Point C(2.5, -3.0);
    C.scale(0.0);

    CHECK(C.getX() == 0.0);
    CHECK(C.getY() == 0.0);

    Point D(2, -3);
    D.scale(-2);

    CHECK(D.getX() == -4);
    CHECK(D.getY() == 6);

    Point E(1.111, 2);
    E.scale(3).scale(2);

    CHECK(E.getX() == 6.666);
    CHECK(E.getY() == 12);

	Point F(1.25, -0.5);  // exact-friendly
    double d = 8.0;

    Point G = F * d;

    F.scale(d);
    CHECK(F == G);
};



TEST_CASE("Testing normalize method", "[Point][normalize]"){
    Point A(3, 4);
    A.normalize();

    CHECK(tol_equal(A.getX(), 0.6));
    CHECK(tol_equal(A.getY(), 0.8));

    CHECK(tol_equal(A.magnitude(), 1.0));

    Point B(-3, -4);
    B.normalize();

    CHECK(tol_equal(B.getX(), -0.6));
    CHECK(tol_equal(B.getY(), -0.8));
    CHECK(tol_equal(B.magnitude(), 1.0));

    Point C(1, 0);
    C.normalize();

    CHECK(tol_equal(C.getX(), 1.0));
    CHECK(tol_equal(C.getY(), 0.0));
    CHECK(tol_equal(C.magnitude(), 1.0));

	// idempotent -- normalize twice
    Point D(3, 4);
    D.normalize();
    Point once = D;

    D.normalize();
    CHECK(D == once); 
    CHECK(tol_equal(D.magnitude(), 1.0));

    Point E(3, 4);
    Point F(3, 4);

    E.normalize();
    F.scale(15).normalize();

    CHECK(E == F);
    CHECK(tol_equal(E.magnitude(), 1.0));
    CHECK(tol_equal(F.magnitude(), 1.0));

    Point P(1.25, -0.5); 
    P.normalize();
    CHECK(tol_equal(A.dot(A), 1.0));

    Point Q(3, 4);
    Q.normalize().scale(5);

    CHECK(tol_equal(Q.getX(), 3.0));
    CHECK(tol_equal(Q.getY(), 4.0));


    Point Z(0, 0);
    Z.normalize();
    CHECK(Z.getX() == 0.0);
    CHECK(Z.getY() == 0.0);
    CHECK(Z.magnitude() == 0.0);
};



TEST_CASE("Testing rotate method", "[Point][rotate]"){

    Point A(2.5, -3.0);
    Point original = A;

    A.rotate(0);
    CHECK(A == original);

    Point B(1,0);
    B.rotate(90);
    CHECK(tol_equal(B.getX() ,  0.0));
    CHECK(tol_equal(B.getY() ,  1.0));


    Point C(1,0);
    C.rotate(90, Point(0,0), false);
    CHECK(tol_equal(C.getX(), 0.0));
    CHECK(tol_equal(C.getY(), -1.0));

    Point D(2,-3);
    D.rotate(180);
    CHECK(tol_equal(D.getX(), -2.0));
    CHECK(tol_equal(D.getY(), 3.0));

    Point E(2.5, -3.0);
    Point ooriginal = E;

    E.rotate(360);
    CHECK(A == ooriginal);

    Point F(2,1);
    Point pivot(1,1);
    F.rotate(90, pivot);
    CHECK(tol_equal(F.getX(), 1.0));
    CHECK(tol_equal(F.getY(), 2.0));

    Point G(2,1);
    G.rotate(90, pivot, false);
    CHECK(tol_equal(G.getX(), 1.0));
    CHECK(tol_equal(G.getY(), 0.0));

    Point H(1,0);
    H.rotate(90).rotate(90);
    CHECK(tol_equal(H.getX(), -1.0));
    CHECK(tol_equal(H.getY(), 0.0));


    Point I(3,4);
    Point pivotI(1,2);
    Point before = I;

    double dx0 = before.getX() - pivotI.getX();
    double dy0 = before.getY() - pivotI.getY();
    double dsquare_before = dx0*dx0 + dy0*dy0;

    I.rotate(37,pivotI);

    double dx1 = I.getX() - pivotI.getX();
    double dy1 = I.getY() - pivotI.getY();
    double dsquare_after = dx1*dx1 + dy1*dy1;

    CHECK(tol_equal(dsquare_before, dsquare_after));



};



TEST_CASE("Testing cross_product method", "[Point][cross_product]"){
   Point A(1,2); 
   Point B(3,4); 

    CHECK(tol_equal(A.cross_product(B), -2.0));
    CHECK(tol_equal(B.cross_product(A), 2.0));

    Point C(5.5, -3.0);
    CHECK(tol_equal(C.cross_product(C), 0.0));

    Point D(4,8);
    CHECK(tol_equal(A.cross_product(D), 0.0));
    CHECK(tol_equal(D.cross_product(A), 0.0));

    Point i(1,0);
    Point j(0,1);
    CHECK(tol_equal(i.cross_product(j), 1.0));
    CHECK(tol_equal(j.cross_product(i), -1.0));

    Point M(1,0);
    Point N(1,1);
    Point P(1,-1);
    CHECK(M.cross_product(N) > 0);
    CHECK(M.cross_product(P) < 0);

    CHECK(tol_equal(A.cross_product(B+C), A.cross_product(B) +
          A.cross_product(C)));
    
    Point E(3,0);
    Point F(0,-4);
    double d = -2.5;

    CHECK(tol_equal((E*d).cross_product(F), d*E.cross_product(F)));
    CHECK(tol_equal((E).cross_product(F*d), d*E.cross_product(F)));

    //triangle
    double res = E.cross_product(F);
    CHECK(tol_equal(res, -12.0));

    double triangle_area = 0.5 * std::abs(res);
    CHECK(tol_equal(triangle_area, 6.0));

};
