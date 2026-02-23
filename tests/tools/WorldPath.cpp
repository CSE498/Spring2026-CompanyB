#include "tools/WorldPath.hpp"
#include "catch2/catch.hpp"
#include <cmath>

using namespace cse498;

// The following tests were generated with AI assistance:
//   - "WorldPath reserve does not change size"
//   - "WorldPath operator[] access"
//   - "WorldPath isClosed with single point"
//   - "WorldPath selfIntersects with fewer than 4 points"
//   - "WorldPath isValid on empty path"
//   - "WorldPath isClosed uses epsilon"
//   - "WorldPath isValid detects NaN after mutation"

TEST_CASE("WorldPath default constructor produces empty path", "[worldpath]") {
  WorldPath path;
  REQUIRE(path.empty());
  REQUIRE(path.size() == 0);
}

TEST_CASE("WorldPath basic mutation", "[worldpath]") {
  WorldPath path;
  REQUIRE(path.empty());

  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 4.0});

  REQUIRE_FALSE(path.empty());
  REQUIRE(path.size() == 2);
  REQUIRE(path.front().x() == Approx(0.0));
  REQUIRE(path.back().y() == Approx(4.0));
}

TEST_CASE("WorldPath clear resets to empty", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});
  path.clear();
  REQUIRE(path.empty());
  REQUIRE(path.size() == 0);
}

TEST_CASE("WorldPath reserve does not change size", "[worldpath]") {
  WorldPath path;
  path.reserve(100);
  REQUIRE(path.empty());
  path.addPoint({0.0, 0.0});
  REQUIRE(path.size() == 1);
}

TEST_CASE("WorldPath operator[] access", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});

  REQUIRE(path[0].x() == Approx(1.0));
  REQUIRE(path[1].y() == Approx(4.0));
}

TEST_CASE("WorldPath at throws out_of_range", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  REQUIRE_THROWS_AS(path.at(1), std::out_of_range);
}

TEST_CASE("WorldPath popBack", "[worldpath]") {
  WorldPath path;
  REQUIRE_FALSE(path.popBack());

  path.addPoint({0.0, 0.0});
  REQUIRE(path.popBack());
  REQUIRE(path.empty());
}

TEST_CASE("WorldPath popBackPoint", "[worldpath]") {
  WorldPath path;
  REQUIRE_FALSE(path.popBackPoint().has_value());

  path.addPoint({0.0, 0.0});
  auto popped = path.popBackPoint();
  REQUIRE(popped.has_value());
  REQUIRE(popped->x() == Approx(0.0));
  REQUIRE(path.empty());
}

TEST_CASE("WorldPath segmentLength and totalLength", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 4.0});
  path.addPoint({6.0, 8.0});

  REQUIRE(path.segmentLength(0).value() == Approx(5.0));
  REQUIRE(path.segmentLength(1).value() == Approx(5.0));
  REQUIRE(path.segmentLength(2) == std::nullopt);
  REQUIRE(path.totalLength() == Approx(10.0));
}

TEST_CASE("WorldPath totalLength on empty and single", "[worldpath]") {
  WorldPath empty;
  REQUIRE(empty.totalLength() == Approx(0.0));

  WorldPath single;
  single.addPoint({2.0, 3.0});
  REQUIRE(single.totalLength() == Approx(0.0));
}

TEST_CASE("WorldPath furthestPair", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({1.0, 0.0});
  path.addPoint({0.0, 2.0});

  auto [a, b] = path.furthestPair();
  double d = std::hypot(b.x() - a.x(), b.y() - a.y());
  REQUIRE(d == Approx(std::hypot(1.0, 2.0)));
}

TEST_CASE("WorldPath furthestPair with two points", "[worldpath]") {
  WorldPath path;
  path.addPoint({-1.0, 0.0});
  path.addPoint({2.0, 0.0});

  auto [a, b] = path.furthestPair();
  REQUIRE(a.x() == Approx(-1.0));
  REQUIRE(b.x() == Approx(2.0));
}

TEST_CASE("WorldPath isClosed", "[worldpath]") {
  WorldPath open;
  open.addPoint({0.0, 0.0});
  open.addPoint({1.0, 0.0});
  REQUIRE_FALSE(open.isClosed());

  WorldPath closed;
  closed.addPoint({0.0, 0.0});
  closed.addPoint({1.0, 0.0});
  closed.addPoint({0.0, 0.0});
  REQUIRE(closed.isClosed());
}

TEST_CASE("WorldPath isClosed with single point", "[worldpath]") {
  WorldPath path;
  path.addPoint({5.0, 5.0});
  REQUIRE_FALSE(path.isClosed());
}

TEST_CASE("WorldPath append and reversed", "[worldpath]") {
  WorldPath a;
  a.addPoint({0.0, 0.0});
  a.addPoint({1.0, 0.0});

  WorldPath b;
  b.addPoint({2.0, 0.0});
  b.addPoint({3.0, 0.0});

  a.append(b);
  REQUIRE(a.size() == 4);
  REQUIRE(a.back().x() == Approx(3.0));

  WorldPath r = a.reversed();
  REQUIRE(r.front().x() == Approx(3.0));
  REQUIRE(r.back().x() == Approx(0.0));
}

TEST_CASE("WorldPath append to empty", "[worldpath]") {
  WorldPath a;
  WorldPath b;
  b.addPoint({1.0, 1.0});
  b.addPoint({2.0, 2.0});

  a.append(b);
  REQUIRE(a.size() == 2);
  REQUIRE(a.front().x() == Approx(1.0));
}

TEST_CASE("WorldPath reversed on single-point path", "[worldpath]") {
  WorldPath path;
  path.addPoint({7.0, -2.0});

  WorldPath r = path.reversed();
  REQUIRE(r.size() == 1);
  REQUIRE(r.front().x() == Approx(7.0));
  REQUIRE(r.front().y() == Approx(-2.0));
}

TEST_CASE("WorldPath pointAtDistance", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({10.0, 0.0});

  REQUIRE(path.pointAtDistance(-1.0).x() == Approx(0.0));
  REQUIRE(path.pointAtDistance(0.0).x() == Approx(0.0));
  REQUIRE(path.pointAtDistance(5.0).x() == Approx(5.0));
  REQUIRE(path.pointAtDistance(15.0).x() == Approx(10.0));
}

TEST_CASE("WorldPath pointAtDistance across segments", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  auto p = path.pointAtDistance(4.0);
  REQUIRE(p.x() == Approx(3.0));
  REQUIRE(p.y() == Approx(1.0));
}

TEST_CASE("WorldPath pointAtDistance handles zero-length segments",
          "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({0.0, 0.0});
  path.addPoint({5.0, 0.0});

  auto p = path.pointAtDistance(2.0);
  REQUIRE(p.x() == Approx(2.0));
  REQUIRE(p.y() == Approx(0.0));
}

TEST_CASE("WorldPath selfIntersects", "[worldpath]") {
  WorldPath nonIntersect;
  nonIntersect.addPoint({0.0, 0.0});
  nonIntersect.addPoint({1.0, 0.0});
  nonIntersect.addPoint({2.0, 0.0});
  REQUIRE_FALSE(nonIntersect.selfIntersects());

  WorldPath intersect;
  intersect.addPoint({0.0, 0.0});
  intersect.addPoint({2.0, 2.0});
  intersect.addPoint({0.0, 2.0});
  intersect.addPoint({2.0, 0.0});
  REQUIRE(intersect.selfIntersects());
}

TEST_CASE("WorldPath selfIntersects closed loop touching", "[worldpath]") {
  WorldPath closed;
  closed.addPoint({0.0, 0.0});
  closed.addPoint({2.0, 0.0});
  closed.addPoint({2.0, 2.0});
  closed.addPoint({0.0, 2.0});
  closed.addPoint({0.0, 0.0});
  REQUIRE_FALSE(closed.selfIntersects());
}

TEST_CASE("WorldPath handles collinear overlap", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({2.0, 0.0});
  path.addPoint({1.0, 0.0});
  path.addPoint({3.0, 0.0});
  REQUIRE(path.selfIntersects());
}

TEST_CASE("WorldPath selfIntersects with fewer than 4 points", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({1.0, 0.0});
  path.addPoint({2.0, 1.0});
  REQUIRE_FALSE(path.selfIntersects());
}

TEST_CASE("WorldPath isValid on valid path", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});
  REQUIRE(path.isValid());
}

TEST_CASE("WorldPath isValid on empty path", "[worldpath]") {
  WorldPath path;
  REQUIRE(path.isValid());
}

TEST_CASE("WorldPath pointsView provides contiguous view", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});

  auto view = path.pointsView();
  REQUIRE(view.size() == 2);
  REQUIRE(view[0].x() == Approx(1.0));
  REQUIRE(view[1].y() == Approx(4.0));
}

TEST_CASE("WorldPath segments yields correct pairs", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({1.0, 0.0});
  path.addPoint({1.0, 2.0});

  auto segs = path.segments();
  auto it = segs.begin();
  REQUIRE(it != segs.end());
  REQUIRE(std::get<0>(*it).x() == Approx(0.0));
  REQUIRE(std::get<1>(*it).x() == Approx(1.0));

  ++it;
  REQUIRE(it != segs.end());
  REQUIRE(std::get<0>(*it).y() == Approx(0.0));
  REQUIRE(std::get<1>(*it).y() == Approx(2.0));
}

TEST_CASE("WorldPath segments on single point path", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 1.0});

  auto segs = path.segments();
  REQUIRE(segs.begin() == segs.end());
}

TEST_CASE("WorldPath segments on empty path", "[worldpath]") {
  WorldPath path;
  auto segs = path.segments();
  REQUIRE(segs.begin() == segs.end());
}

TEST_CASE("WorldPath reversed on empty path", "[worldpath]") {
  WorldPath path;
  WorldPath r = path.reversed();
  REQUIRE(r.empty());
  REQUIRE(r.size() == 0);
}

TEST_CASE("WorldPath isClosed on empty path", "[worldpath]") {
  WorldPath path;
  REQUIRE_FALSE(path.isClosed());
}

TEST_CASE("WorldPath furthestPair with overlapping points", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 1.0});
  path.addPoint({1.0, 1.0});

  auto [a, b] = path.furthestPair();
  REQUIRE(a.x() == Approx(1.0));
  REQUIRE(b.x() == Approx(1.0));
}

TEST_CASE("WorldPath pointAtDistance hits exact vertex", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  auto p = path.pointAtDistance(3.0);
  REQUIRE(p.x() == Approx(3.0));
  REQUIRE(p.y() == Approx(0.0));
}

TEST_CASE("WorldPath isClosed uses epsilon", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({1.0, 0.0});
  path.addPoint({1e-10, 0.0});

  REQUIRE(path.isClosed());
  REQUIRE_FALSE(path.isClosed(1e-12));
}

TEST_CASE("WorldPath isValid detects NaN after mutation", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({1.0, 1.0});

  path[1] = {std::nan(""), 1.0};
  REQUIRE_FALSE(path.isValid());
}

TEST_CASE("WorldPath selfIntersects counts vertex touch", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({2.0, 0.0});
  path.addPoint({2.0, 2.0});
  path.addPoint({0.0, 0.0}); // returns to start (touch)
  path.addPoint({0.0, 2.0}); // continues, should still count as intersecting

  REQUIRE(path.selfIntersects());
}
