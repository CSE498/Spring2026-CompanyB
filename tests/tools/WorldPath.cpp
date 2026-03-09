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
//
// The following modifications were made with AI assistance:
//   - Restructured all TEST_CASEs into SECTION-based groupings
//   - Added GENERATE/CAPTURE for parametric small-path tests


TEST_CASE("WorldPath basic container behavior", "[worldpath]") {
  WorldPath path;

  SECTION("default constructed path is empty") {
    REQUIRE(path.empty());
    REQUIRE(path.size() == 0);
  }

  SECTION("adding points updates size and endpoints") {
    path.addPoint({0.0, 0.0});
    path.addPoint({3.0, 4.0});

    REQUIRE_FALSE(path.empty());
    REQUIRE(path.size() == 2);
    REQUIRE(path.front().x() == Approx(0.0));
    REQUIRE(path.back().y() == Approx(4.0));
  }

  SECTION("reserve does not change logical size") {
    path.reserve(100);
    REQUIRE(path.empty());
    REQUIRE(path.size() == 0);

    path.addPoint({0.0, 0.0});
    REQUIRE(path.size() == 1);
  }

  SECTION("clear resets path and allows reuse") {
    path.addPoint({1.0, 2.0});
    path.addPoint({3.0, 4.0});
    path.clear();

    REQUIRE(path.empty());
    REQUIRE(path.size() == 0);

    path.addPoint({9.0, 8.0});
    REQUIRE(path.size() == 1);
    REQUIRE(path.front().x() == Approx(9.0));
    REQUIRE(path.front().y() == Approx(8.0));
  }
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

  SECTION("valid index returns the point") {
    REQUIRE(path.at(0).x() == Approx(0.0));
  }

  SECTION("out-of-range index throws") {
    REQUIRE_THROWS_AS(path.at(1), std::out_of_range);
  }
}


TEST_CASE("WorldPath popBack", "[worldpath]") {
  WorldPath path;

  SECTION("empty path returns nullopt") {
    REQUIRE_FALSE(path.popBack().has_value());
  }

  SECTION("non-empty path returns and removes the last point") {
    path.addPoint({5.0, 6.0});
    auto popped = path.popBack();
    REQUIRE(popped.has_value());
    REQUIRE(popped->x() == Approx(5.0));
    REQUIRE(popped->y() == Approx(6.0));
    REQUIRE(path.empty());
  }
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


TEST_CASE("WorldPath segments", "[worldpath]") {
  WorldPath path;

  SECTION("empty path has no segments") {
    auto segs = path.segments();
    REQUIRE(segs.begin() == segs.end());
  }

  SECTION("single-point path has no segments") {
    path.addPoint({1.0, 1.0});
    auto segs = path.segments();
    REQUIRE(segs.begin() == segs.end());
  }

  SECTION("multi-point path yields consecutive pairs") {
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

    ++it;
    REQUIRE(it == segs.end());
  }
}


TEST_CASE("WorldPath totalLength", "[worldpath]") {
  SECTION("zero for fewer than two points") {
    const int point_count = GENERATE(0, 1);
    CAPTURE(point_count);

    WorldPath path;
    for (int i = 0; i < point_count; ++i) {
      path.addPoint({static_cast<double>(i), static_cast<double>(i + 1)});
    }

    REQUIRE(path.totalLength() == Approx(0.0));
  }

  SECTION("sums segment distances") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({3.0, 4.0});
    path.addPoint({6.0, 8.0});

    REQUIRE(path.totalLength() == Approx(10.0));
  }
}

TEST_CASE("WorldPath segmentLength", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 4.0});
  path.addPoint({6.0, 8.0});

  SECTION("valid segment indices") {
    REQUIRE(path.segmentLength(0).value() == Approx(5.0));
    REQUIRE(path.segmentLength(1).value() == Approx(5.0));
  }

  SECTION("out-of-range returns nullopt") {
    REQUIRE_FALSE(path.segmentLength(2).has_value());
  }
}


TEST_CASE("WorldPath furthestPair", "[worldpath]") {
  SECTION("three points returns the most distant pair") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({0.0, 2.0});

    auto [a, b] = path.furthestPair();
    double d = std::hypot(b.x() - a.x(), b.y() - a.y());
    REQUIRE(d == Approx(std::hypot(1.0, 2.0)));
  }

  SECTION("two points returns those two") {
    WorldPath path;
    path.addPoint({-1.0, 0.0});
    path.addPoint({2.0, 0.0});

    auto [a, b] = path.furthestPair();
    REQUIRE(a.x() == Approx(-1.0));
    REQUIRE(b.x() == Approx(2.0));
  }

  SECTION("overlapping points") {
    WorldPath path;
    path.addPoint({1.0, 1.0});
    path.addPoint({1.0, 1.0});

    auto [a, b] = path.furthestPair();
    double d = std::hypot(b.x() - a.x(), b.y() - a.y());
    REQUIRE(d == Approx(0.0));
  }
}


TEST_CASE("WorldPath isClosed", "[worldpath]") {
  SECTION("empty path is not closed") {
    WorldPath path;
    REQUIRE_FALSE(path.isClosed());
  }

  SECTION("single-point path is not closed") {
    WorldPath path;
    path.addPoint({5.0, 5.0});
    REQUIRE_FALSE(path.isClosed());
  }

  SECTION("distinct endpoints are not closed") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    REQUIRE_FALSE(path.isClosed());
  }

  SECTION("matching endpoints are closed") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({0.0, 0.0});
    REQUIRE(path.isClosed());
  }

  SECTION("epsilon affects closure") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({1e-10, 0.0});

    REQUIRE(path.isClosed());
    REQUIRE_FALSE(path.isClosed(1e-12));
  }
}


TEST_CASE("WorldPath append", "[worldpath]") {
  SECTION("append to non-empty path") {
    WorldPath a;
    a.addPoint({0.0, 0.0});
    a.addPoint({1.0, 0.0});

    WorldPath b;
    b.addPoint({2.0, 0.0});
    b.addPoint({3.0, 0.0});

    a.append(b);
    REQUIRE(a.size() == 4);
    REQUIRE(a.back().x() == Approx(3.0));
  }

  SECTION("append to empty path") {
    WorldPath a;
    WorldPath b;
    b.addPoint({1.0, 1.0});
    b.addPoint({2.0, 2.0});

    a.append(b);
    REQUIRE(a.size() == 2);
    REQUIRE(a.front().x() == Approx(1.0));
  }
}

TEST_CASE("WorldPath reversed", "[worldpath]") {
  SECTION("empty path") {
    WorldPath path;
    WorldPath r = path.reversed();
    REQUIRE(r.empty());
  }

  SECTION("single-point path") {
    WorldPath path;
    path.addPoint({7.0, -2.0});

    WorldPath r = path.reversed();
    REQUIRE(r.size() == 1);
    REQUIRE(r.front().x() == Approx(7.0));
    REQUIRE(r.front().y() == Approx(-2.0));
  }

  SECTION("multi-point path") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({2.0, 0.0});
    path.addPoint({3.0, 0.0});

    WorldPath r = path.reversed();
    REQUIRE(r.front().x() == Approx(3.0));
    REQUIRE(r.back().x() == Approx(0.0));
  }
}


TEST_CASE("WorldPath pointAtDistance", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  SECTION("clamps before start") {
    auto p = path.pointAtDistance(-1.0);
    REQUIRE(p.x() == Approx(0.0));
    REQUIRE(p.y() == Approx(0.0));
  }

  SECTION("returns exact start") {
    auto p = path.pointAtDistance(0.0);
    REQUIRE(p.x() == Approx(0.0));
    REQUIRE(p.y() == Approx(0.0));
  }

  SECTION("interpolates inside first segment") {
    auto p = path.pointAtDistance(2.0);
    REQUIRE(p.x() == Approx(2.0));
    REQUIRE(p.y() == Approx(0.0));
  }

  SECTION("hits exact vertex") {
    auto p = path.pointAtDistance(3.0);
    REQUIRE(p.x() == Approx(3.0));
    REQUIRE(p.y() == Approx(0.0));
  }

  SECTION("interpolates across second segment") {
    auto p = path.pointAtDistance(4.0);
    REQUIRE(p.x() == Approx(3.0));
    REQUIRE(p.y() == Approx(1.0));
  }

  SECTION("clamps after end") {
    auto p = path.pointAtDistance(100.0);
    REQUIRE(p.x() == Approx(3.0));
    REQUIRE(p.y() == Approx(4.0));
  }

  SECTION("handles zero-length segments") {
    WorldPath dup;
    dup.addPoint({0.0, 0.0});
    dup.addPoint({0.0, 0.0});
    dup.addPoint({5.0, 0.0});

    auto p = dup.pointAtDistance(2.0);
    REQUIRE(p.x() == Approx(2.0));
    REQUIRE(p.y() == Approx(0.0));
  }
}


TEST_CASE("WorldPath selfIntersects is false for small paths", "[worldpath]") {
  const int point_count = GENERATE(0, 1, 2, 3);
  CAPTURE(point_count);

  WorldPath path;
  for (int i = 0; i < point_count; ++i) {
    path.addPoint({static_cast<double>(i), 0.0});
  }

  REQUIRE_FALSE(path.selfIntersects());
}

TEST_CASE("WorldPath selfIntersects", "[worldpath]") {
  SECTION("collinear non-overlapping does not intersect") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({2.0, 0.0});
    REQUIRE_FALSE(path.selfIntersects());
  }

  SECTION("X-crossing path intersects") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({2.0, 2.0});
    path.addPoint({0.0, 2.0});
    path.addPoint({2.0, 0.0});
    REQUIRE(path.selfIntersects());
  }

  SECTION("closed non-crossing loop does not intersect") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({2.0, 0.0});
    path.addPoint({2.0, 2.0});
    path.addPoint({0.0, 2.0});
    path.addPoint({0.0, 0.0});
    REQUIRE_FALSE(path.selfIntersects());
  }

  SECTION("collinear overlap intersects") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({2.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({3.0, 0.0});
    REQUIRE(path.selfIntersects());
  }

  SECTION("vertex touch counts as intersection") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({2.0, 0.0});
    path.addPoint({2.0, 2.0});
    path.addPoint({0.0, 0.0});  // returns to start
    path.addPoint({0.0, 2.0});  // continues
    REQUIRE(path.selfIntersects());
  }
}


TEST_CASE("WorldPath isValid", "[worldpath]") {
  SECTION("empty path is valid") {
    WorldPath path;
    REQUIRE(path.isValid());
  }

  SECTION("path with finite points is valid") {
    WorldPath path;
    path.addPoint({1.0, 2.0});
    path.addPoint({3.0, 4.0});
    REQUIRE(path.isValid());
  }

  SECTION("NaN after mutation is invalid") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 1.0});

    path[1] = {std::nan(""), 1.0};
    REQUIRE_FALSE(path.isValid());
  }
}
