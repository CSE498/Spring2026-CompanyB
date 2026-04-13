#include <catch2/catch_test_macros.hpp>

#include "../../source/tools/Point.hpp"

using namespace cse498;

TEST_CASE("Testing Constructors, setters/getters", "[Point][constructor]") {
  Point A;
  CHECK(A.getX() == 0);
  CHECK(A.getY() == 0);

  Point B(11.3, 15.7);
  CHECK(B.getX() == 11.3);
  CHECK(B.getY() == 15.7);

  A = Point(8, 51.6);
  CHECK(A.getX() == 8);
  CHECK(A.getY() == 51.6);

  B.setY(7).setX(0);
  CHECK(B.getY() == 7);
  CHECK(B.getX() == 0);

  A.setX(21);
  CHECK(A.getX() == 21);
};

TEST_CASE("Testing addition operator", "[Point][operator+]") {
  Point A(12, 13);
  Point B(1.2, 7.8);
  Point C;

  C = A + B;
  CHECK(tol_equal(C.getX(), 13.2));
  CHECK(tol_equal(C.getY(), 20.8));

  CHECK(tol_equal((A + B).getX(), 13.2));
  double scalar1 = 2.7;
  Point D(14.7, 15.7);
  CHECK((A + scalar1) == D);

  Point Z(12, 13);
  double s = 2.7;

  Point left = s + Z;
  Point right = Z + s;

  CHECK(tol_equal(left.getX(), right.getX()));
  CHECK(tol_equal(left.getY(), right.getY()));

  // Expected numeric result
  CHECK(tol_equal(left.getX(), 14.7));
  CHECK(tol_equal(left.getY(), 15.7));

  Point M(5.5, -3.0);
  Point zero(0, 0);

  Point N = M + zero;
  CHECK(N == M);

  Point K = M + 0.0;
  CHECK(K == M);

  Point R = 0.0 + M;
  CHECK(R == M);
};

TEST_CASE("Testing subtraction operator", "[Point][operator-]") {
  Point A(12, 13);
  Point B(1.2, 7.8);

  Point C = A - B;
  CHECK(tol_equal(C.getX(), 10.8));
  CHECK(tol_equal(C.getY(), 5.2));

  // A - A = (0,0)
  Point Z = A - A;
  CHECK(Z.getX() == 0);
  CHECK(Z.getY() == 0);

  double s = 2.7;

  Point M = A - s;
  CHECK(tol_equal(M.getX(), 9.3));
  CHECK(tol_equal(M.getY(), 10.3));

  Point N = s - A;
  CHECK(tol_equal(N.getX(), 2.7 - 12));
  CHECK(tol_equal(N.getY(), 2.7 - 13));

  Point Q(12, 13);
  CHECK((Q - s) == (Q + (-s)));
};

TEST_CASE("Testing multiplication operator", "[Point][operator*]") {
  Point A(2, 3);
  Point B(4, 5);

  Point C = A * B;
  CHECK(C.getX() == 8);
  CHECK(C.getY() == 15);

  Point M(2, -3);
  double s = 2.76;

  Point D = M * s;
  CHECK(tol_equal(D.getX(), 5.52));
  CHECK(tol_equal(D.getY(), -8.28));

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

TEST_CASE("Testing equality operator", "[Point][equal==]") {
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

  CHECK(A != s);
  CHECK(s != A);

  CHECK_FALSE(A == s);
  CHECK_FALSE(s == A);
};

TEST_CASE("Testing dot method", "[Point][dot]") {
  Point A(-1.3, 5);
  Point B(15.1, 7.12);
  Point E(15.1, 7.12);

  CHECK(tol_equal(A.dot(A), 26.69));
  CHECK(tol_equal(B.dot(B), E.dot(E)));

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

  CHECK(tol_equal(M.dot(N + O), M.dot(N) + M.dot(O)));
};

TEST_CASE("Testing magnitude method", "[Point][magnitude]") {
  Point Z(0, 0);
  CHECK(Z.magnitude() == 0.0);

  Point A(3, 4);
  CHECK(A.magnitude() == 5.0);

  Point B(-3, -4);
  CHECK(B.magnitude() == 5.0);

  Point X(7, 0);
  CHECK(X.magnitude() == 7.0);

  Point Y(0, -2);
  CHECK(Y.magnitude() == 2.0);

  Point M(0.5, 0.0);
  CHECK(M.magnitude() == 0.5);

  Point Q(0.5, 0.25);
  double expected = std::sqrt(5.0) / 4.0;

  CHECK(tol_equal(Q.magnitude(), expected));

  CHECK(tol_equal(A.magnitude() * A.magnitude(), A.dot(A)));
  CHECK(tol_equal(Q.magnitude() * Q.magnitude(), Q.dot(Q)));
};

TEST_CASE("Testing scale method", "[Point][scale]") {
  Point A(2, 3);
  A.scale(4);

  CHECK(A.getX() == 8);
  CHECK(A.getY() == 12);

  Point B(2.5, -3.0);
  Point original = B;

  B.scale(1.0);
  CHECK(B == original);

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

  CHECK(tol_equal(E.getX(), 6.666));
  CHECK(E.getY() == 12);

  Point F(1.25, -0.5);
  double d = 8.0;

  Point G = F * d;

  F.scale(d);
  CHECK(F == G);
};

TEST_CASE("Testing scaled method", "[Point][scaled]") {
  Point A(2, 3);
  Point B = A.scaled(4);

  // original unchanged
  CHECK(A.getX() == 2);
  CHECK(A.getY() == 3);

  // scaled copy is correct
  CHECK(B.getX() == 8);
  CHECK(B.getY() == 12);

  // scaled(1) returns equal copy
  CHECK(A.scaled(1.0) == A);

  // scaled(0) returns origin
  Point Z = A.scaled(0.0);
  CHECK(Z.getX() == 0.0);
  CHECK(Z.getY() == 0.0);

  // scaled should match mutating scale
  Point C(1.25, -0.5);
  Point D = C;
  D.scale(3.0);
  CHECK(C.scaled(3.0) == D);
};

TEST_CASE("Testing normalize method", "[Point][normalize]") {
  auto result = Point::normalize(Point(3, 4));
  REQUIRE(result.has_value());
  CHECK(tol_equal(result->getX(), 0.6));
  CHECK(tol_equal(result->getY(), 0.8));
  CHECK(tol_equal(result->magnitude(), 1.0));

  auto neg = Point::normalize(Point(-3, -4));
  REQUIRE(neg.has_value());
  CHECK(tol_equal(neg->getX(), -0.6));
  CHECK(tol_equal(neg->getY(), -0.8));
  CHECK(tol_equal(neg->magnitude(), 1.0));

  auto unit = Point::normalize(Point(1, 0));
  REQUIRE(unit.has_value());
  CHECK(tol_equal(unit->getX(), 1.0));
  CHECK(tol_equal(unit->getY(), 0.0));

  // idempotent
  auto once = Point::normalize(Point(3, 4));
  REQUIRE(once.has_value());
  auto twice = Point::normalize(*once);
  REQUIRE(twice.has_value());
  CHECK(*once == *twice);

  // direction preserved after scaling
  auto a = Point::normalize(Point(3, 4));
  auto b = Point::normalize(Point(3, 4).scaled(15));
  REQUIRE(a.has_value());
  REQUIRE(b.has_value());
  CHECK(*a == *b);

  // dot with self == 1
  auto p = Point::normalize(Point(1.25, -0.5));
  REQUIRE(p.has_value());
  CHECK(tol_equal(p->dot(*p), 1.0));

  // normalize then scale back
  auto q = Point::normalize(Point(3, 4));
  REQUIRE(q.has_value());
  Point restored = q->scaled(5);
  CHECK(tol_equal(restored.getX(), 3.0));
  CHECK(tol_equal(restored.getY(), 4.0));

  // zero vector returns error
  auto zero = Point::normalize(Point(0, 0));
  CHECK_FALSE(zero.has_value());
};

TEST_CASE("Testing rotate method", "[Point][rotate]") {
  Point A(2.5, -3.0);
  Point original = A;

  A.rotate(0);
  CHECK(A == original);

  Point B(1, 0);
  B.rotate(90);
  CHECK(tol_equal(B.getX(), 0.0));
  CHECK(tol_equal(B.getY(), 1.0));

  Point C(1, 0);
  C.rotate(90, Point(0, 0), false);
  CHECK(tol_equal(C.getX(), 0.0));
  CHECK(tol_equal(C.getY(), -1.0));

  Point D(2, -3);
  D.rotate(180);
  CHECK(tol_equal(D.getX(), -2.0));
  CHECK(tol_equal(D.getY(), 3.0));

  Point E(2.5, -3.0);
  Point E_original = E;
  E.rotate(360);
  CHECK(tol_equal(E.getX(), E_original.getX()));
  CHECK(tol_equal(E.getY(), E_original.getY()));

  Point F(2, 1);
  Point pivot(1, 1);
  F.rotate(90, pivot);
  CHECK(tol_equal(F.getX(), 1.0));
  CHECK(tol_equal(F.getY(), 2.0));

  Point G(2, 1);
  G.rotate(90, pivot, false);
  CHECK(tol_equal(G.getX(), 1.0));
  CHECK(tol_equal(G.getY(), 0.0));

  Point H(1, 0);
  H.rotate(90).rotate(90);
  CHECK(tol_equal(H.getX(), -1.0));
  CHECK(tol_equal(H.getY(), 0.0));

  Point I(3, 4);
  Point pivotI(1, 2);
  Point before = I;

  double dx0 = before.getX() - pivotI.getX();
  double dy0 = before.getY() - pivotI.getY();
  double dsquare_before = dx0 * dx0 + dy0 * dy0;

  I.rotate(37, pivotI);

  double dx1 = I.getX() - pivotI.getX();
  double dy1 = I.getY() - pivotI.getY();
  double dsquare_after = dx1 * dx1 + dy1 * dy1;

  CHECK(tol_equal(dsquare_before, dsquare_after));
};

TEST_CASE("Testing rotated method", "[Point][rotated]") {
  Point A(1, 0);
  Point B = A.rotated(90);

  // original unchanged
  CHECK(A.getX() == 1);
  CHECK(A.getY() == 0);

  // rotated copy correct
  CHECK(tol_equal(B.getX(), 0.0));
  CHECK(tol_equal(B.getY(), 1.0));

  // rotated(0) returns equal copy
  Point C(2.5, -3.0);
  CHECK(C.rotated(0) == C);

  // rotated with pivot
  Point D(2, 1);
  Point pivot(1, 1);
  Point E = D.rotated(90, pivot);
  CHECK(tol_equal(E.getX(), 1.0));
  CHECK(tol_equal(E.getY(), 2.0));
  CHECK(D.getX() == 2);  // D unchanged

  // clockwise
  Point F = D.rotated(90, pivot, false);
  CHECK(tol_equal(F.getX(), 1.0));
  CHECK(tol_equal(F.getY(), 0.0));
};

TEST_CASE("Testing cross_product method", "[Point][cross_product]") {
  Point A(1, 2);
  Point B(3, 4);

  CHECK(tol_equal(A.cross_product(B), -2.0));
  CHECK(tol_equal(B.cross_product(A), 2.0));

  Point C(5.5, -3.0);
  CHECK(tol_equal(C.cross_product(C), 0.0));

  Point D(4, 8);
  CHECK(tol_equal(A.cross_product(D), 0.0));
  CHECK(tol_equal(D.cross_product(A), 0.0));

  Point i(1, 0);
  Point j(0, 1);
  CHECK(tol_equal(i.cross_product(j), 1.0));
  CHECK(tol_equal(j.cross_product(i), -1.0));

  Point M(1, 0);
  Point N(1, 1);
  Point P(1, -1);
  CHECK(M.cross_product(N) > 0);
  CHECK(M.cross_product(P) < 0);

  CHECK(tol_equal(A.cross_product(B + C),
                  A.cross_product(B) + A.cross_product(C)));

  Point E(3, 0);
  Point F(0, -4);
  double d = -2.5;

  CHECK(tol_equal((E * d).cross_product(F), d * E.cross_product(F)));
  CHECK(tol_equal((E).cross_product(F * d), d * E.cross_product(F)));

  double res = E.cross_product(F);
  CHECK(tol_equal(res, -12.0));

  double triangle_area = 0.5 * std::abs(res);
  CHECK(tol_equal(triangle_area, 6.0));
};

TEST_CASE("Testing lengthSq method", "[Point][lengthSq]") {
  Point Z(0, 0);
  CHECK(Z.lengthSq() == 0.0);

  Point A(3, 4);
  CHECK(A.lengthSq() == 25.0);

  // lengthSq should equal magnitude squared
  Point B(1.5, -2.3);
  CHECK(tol_equal(B.lengthSq(), B.magnitude() * B.magnitude()));

  // lengthSq should equal dot with self
  CHECK(tol_equal(B.lengthSq(), B.dot(B)));
};

TEST_CASE("Testing distanceTo and distanceSqTo", "[Point][distance]") {
  Point A(0, 0);
  Point B(3, 4);

  CHECK(tol_equal(A.distanceTo(B), 5.0));
  CHECK(tol_equal(A.distanceSqTo(B), 25.0));

  // distance to self is 0
  CHECK(A.distanceTo(A) == 0.0);
  CHECK(A.distanceSqTo(A) == 0.0);

  // symmetry
  CHECK(tol_equal(A.distanceTo(B), B.distanceTo(A)));

  // distanceSq should be distanceTo squared
  Point C(1.5, -2.3);
  Point D(4.1, 0.7);
  double dist = C.distanceTo(D);
  CHECK(tol_equal(C.distanceSqTo(D), dist * dist));
};

TEST_CASE("Testing translated method", "[Point][translated]") {
  Point A(1, 2);
  Point delta(3, -1);

  Point B = A.translated(delta);
  CHECK(tol_equal(B.getX(), 4.0));
  CHECK(tol_equal(B.getY(), 1.0));

  // original unchanged
  CHECK(A.getX() == 1);
  CHECK(A.getY() == 2);

  // translate by zero
  Point Z(0, 0);
  CHECK(A.translated(Z) == A);
};

TEST_CASE("Testing perpendicular method", "[Point][perpendicular]") {
  Point A(1, 0);
  Point P = A.perpendicular();
  CHECK(tol_equal(P.getX(), 0.0));
  CHECK(tol_equal(P.getY(), 1.0));

  // perpendicular should be orthogonal (dot == 0)
  Point B(3, 4);
  CHECK(tol_equal(B.dot(B.perpendicular()), 0.0));

  // same magnitude
  CHECK(tol_equal(B.magnitude(), B.perpendicular().magnitude()));
};

TEST_CASE("Testing angle method", "[Point][angle]") {
  Point right(1, 0);
  CHECK(tol_equal(right.angle(), 0.0));

  Point up(0, 1);
  CHECK(tol_equal(up.angle(), std::numbers::pi / 2.0));

  Point left(-1, 0);
  CHECK(tol_equal(left.angle(), std::numbers::pi));

  Point down(0, -1);
  CHECK(tol_equal(down.angle(), -std::numbers::pi / 2.0));
};

TEST_CASE("Testing lerp method", "[Point][lerp]") {
  Point A(0, 0);
  Point B(10, 20);

  // t=0 returns A
  CHECK(A.lerp(B, 0.0) == A);

  // t=1 returns B
  Point at1 = A.lerp(B, 1.0);
  CHECK(tol_equal(at1.getX(), 10.0));
  CHECK(tol_equal(at1.getY(), 20.0));

  // t=0.5 returns midpoint
  Point mid = A.lerp(B, 0.5);
  CHECK(tol_equal(mid.getX(), 5.0));
  CHECK(tol_equal(mid.getY(), 10.0));

  // extrapolation t=2
  Point ext = A.lerp(B, 2.0);
  CHECK(tol_equal(ext.getX(), 20.0));
  CHECK(tol_equal(ext.getY(), 40.0));
};

TEST_CASE("Testing fromPolar method", "[Point][fromPolar]") {
  // r=1, theta=0 -> (1, 0)
  Point A = Point::fromPolar(1.0, 0.0);
  CHECK(tol_equal(A.getX(), 1.0));
  CHECK(tol_equal(A.getY(), 0.0));

  // r=1, theta=pi/2 -> (0, 1)
  Point B = Point::fromPolar(1.0, std::numbers::pi / 2.0);
  CHECK(tol_equal(B.getX(), 0.0));
  CHECK(tol_equal(B.getY(), 1.0));

  // r=5, theta=0 -> (5, 0)
  Point C = Point::fromPolar(5.0, 0.0);
  CHECK(tol_equal(C.getX(), 5.0));
  CHECK(tol_equal(C.getY(), 0.0));

  // magnitude should equal r
  double r = 3.7;
  double theta = 1.2;
  Point D = Point::fromPolar(r, theta);
  CHECK(tol_equal(D.magnitude(), r));

  // angle should equal theta
  CHECK(tol_equal(D.angle(), theta));

  // r=0 -> origin
  Point Z = Point::fromPolar(0.0, 2.0);
  CHECK(tol_equal(Z.getX(), 0.0));
  CHECK(tol_equal(Z.getY(), 0.0));
};
