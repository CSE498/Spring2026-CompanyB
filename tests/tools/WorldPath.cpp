#include "tools/WorldPath.hpp"
// #include "catch2/catch.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// for Catch::Matchers::WithinRel() for approx double in Catchv3
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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
    REQUIRE_THAT(path.front().getX(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(path.back().getY(), Catch::Matchers::WithinRel(4.0));
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
    REQUIRE_THAT(path.front().getX(), Catch::Matchers::WithinRel(9.0));
    REQUIRE_THAT(path.front().getY(), Catch::Matchers::WithinRel(8.0));
  }

  SECTION("heavy sequential insertions and clearing cycle") {
    for (int i = 0; i < 50; ++i) {
      path.addPoint({static_cast<double>(i), static_cast<double>(i * 2)});
    }
    REQUIRE(path.size() == 50);

    path.clear();
    REQUIRE(path.empty());
    REQUIRE(path.size() == 0);

    path.reserve(100);
    for (int i = 0; i < 50; ++i) {
      path.addPoint({static_cast<double>(-i), static_cast<double>(-i * 2)});
    }

    REQUIRE(path.size() == 50);
    REQUIRE_THAT(path.front().getX(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(path.back().getX(), Catch::Matchers::WithinRel(-49.0));

    // Verify iterator state manually
    std::size_t count = 0;
    for (auto it = path.begin(); it != path.end(); ++it) {
      count++;
    }
    REQUIRE(count == 50);
  }
}

TEST_CASE("WorldPath operator[] access", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});

  REQUIRE_THAT(path[0].getX(), Catch::Matchers::WithinRel(1.0));
  REQUIRE_THAT(path[1].getY(), Catch::Matchers::WithinRel(4.0));
}

TEST_CASE("WorldPath get checked access", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});

  SECTION("valid index returns non-null pointer") {
    auto* p = path.get(0);
    REQUIRE(p != nullptr);
    REQUIRE_THAT(p->getX(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("out-of-range index returns nullptr") {
    REQUIRE(path.get(1) == nullptr);
  }

  SECTION("const overload works") {
    const WorldPath& cpath = path;
    const auto* cp = cpath.get(0);
    REQUIRE(cp != nullptr);
    REQUIRE_THAT(cp->getX(), Catch::Matchers::WithinRel(0.0));
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
    REQUIRE_THAT(popped->getX(), Catch::Matchers::WithinRel(5.0));
    REQUIRE_THAT(popped->getY(), Catch::Matchers::WithinRel(6.0));
    REQUIRE(path.empty());
  }

  SECTION("multiple pops return correct values in LIFO order") {
    path.addPoint({1.0, 2.0});
    path.addPoint({3.0, 4.0});
    path.addPoint({5.0, 6.0});

    auto p3 = path.popBack();
    REQUIRE_THAT(p3->getX(), Catch::Matchers::WithinRel(5.0));
    auto p2 = path.popBack();
    REQUIRE_THAT(p2->getX(), Catch::Matchers::WithinRel(3.0));
    auto p1 = path.popBack();
    REQUIRE_THAT(p1->getX(), Catch::Matchers::WithinRel(1.0));
    REQUIRE(path.empty());
    REQUIRE_FALSE(path.popBack().has_value());
  }

  SECTION("popping heavily populated paths completely") {
    for (int i = 0; i < 100; ++i) {
      path.addPoint({static_cast<double>(i), 0.0});
    }
    REQUIRE(path.size() == 100);

    for (int i = 99; i >= 0; --i) {
      auto popped = path.popBack();
      REQUIRE(popped.has_value());
      REQUIRE_THAT(popped->getX(),
                   Catch::Matchers::WithinRel(static_cast<double>(i)));
    }
    REQUIRE(path.empty());
    REQUIRE_FALSE(path.popBack().has_value());
  }
}

TEST_CASE("WorldPath pointsView provides contiguous view", "[worldpath]") {
  WorldPath path;
  path.addPoint({1.0, 2.0});
  path.addPoint({3.0, 4.0});

  auto view = path.pointsView();
  REQUIRE(view.size() == 2);
  REQUIRE_THAT(view[0].getX(), Catch::Matchers::WithinRel(1.0));
  REQUIRE_THAT(view[1].getY(), Catch::Matchers::WithinRel(4.0));
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
    REQUIRE_THAT(std::get<0>(*it).getX(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(std::get<1>(*it).getX(), Catch::Matchers::WithinRel(1.0));

    ++it;
    REQUIRE(it != segs.end());
    REQUIRE_THAT(std::get<0>(*it).getY(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(std::get<1>(*it).getY(), Catch::Matchers::WithinRel(2.0));

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

    REQUIRE_THAT(path.totalLength(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("sums segment distances") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({3.0, 4.0});
    path.addPoint({6.0, 8.0});

    REQUIRE_THAT(path.totalLength(), Catch::Matchers::WithinRel(10.0));
  }

  SECTION("long zigzag path length matches manual accumulation") {
    WorldPath p;
    double manual_length = 0.0;
    Point current{0.0, 0.0};
    p.addPoint(current);

    for (int i = 1; i <= 20; ++i) {
      Point next{static_cast<double>(i * 3),
                 static_cast<double>((i % 2 == 0) ? 4 : -4)};
      manual_length += std::hypot(next.getX() - current.getX(),
                                  next.getY() - current.getY());
      p.addPoint(next);
      current = next;
    }

    REQUIRE_THAT(p.totalLength(), Catch::Matchers::WithinRel(manual_length));
    REQUIRE(p.totalLength() > 0.0);
  }
}

TEST_CASE("WorldPath segmentLengthAt", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 4.0});
  path.addPoint({6.0, 8.0});

  SECTION("valid segment indices") {
    REQUIRE_THAT(path.segmentLengthAt(0).value(),
                 Catch::Matchers::WithinRel(5.0));
    REQUIRE_THAT(path.segmentLengthAt(1).value(),
                 Catch::Matchers::WithinRel(5.0));
  }

  SECTION("out-of-range returns nullopt") {
    REQUIRE_FALSE(path.segmentLengthAt(2).has_value());
  }
}

TEST_CASE("WorldPath subpathLength", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  SECTION("valid indices") {
    REQUIRE_THAT(path.subpathLength(0, 2).value(),
                 Catch::Matchers::WithinRel(7.0));
    REQUIRE_THAT(path.subpathLength(0, 1).value(),
                 Catch::Matchers::WithinRel(3.0));
    REQUIRE_THAT(path.subpathLength(1, 2).value(),
                 Catch::Matchers::WithinRel(4.0));
    REQUIRE_THAT(path.subpathLength(1, 1).value(),
                 Catch::Matchers::WithinRel(0.0));
  }

  SECTION("invalid indices") {
    REQUIRE_FALSE(path.subpathLength(0, 4).has_value());
    REQUIRE_FALSE(path.subpathLength(2, 0).has_value());  // backward queries
  }
}

TEST_CASE("WorldPath furthestPair", "[worldpath]") {
  SECTION("three points returns the most distant pair") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({0.0, 2.0});

    auto [a, b] = path.furthestPair();
    double d = std::hypot(b.getX() - a.getX(), b.getY() - a.getY());
    REQUIRE_THAT(d, Catch::Matchers::WithinRel(std::hypot(1.0, 2.0)));
  }

  SECTION("two points returns those two") {
    WorldPath path;
    path.addPoint({-1.0, 0.0});
    path.addPoint({2.0, 0.0});

    auto [a, b] = path.furthestPair();
    REQUIRE_THAT(a.getX(), Catch::Matchers::WithinRel(-1.0));
    REQUIRE_THAT(b.getX(), Catch::Matchers::WithinRel(2.0));
  }

  SECTION("overlapping points") {
    WorldPath path;
    path.addPoint({1.0, 1.0});
    path.addPoint({1.0, 1.0});

    auto [a, b] = path.furthestPair();
    double d = std::hypot(b.getX() - a.getX(), b.getY() - a.getY());
    REQUIRE_THAT(d, Catch::Matchers::WithinRel(0.0));
  }

  SECTION("polygon shape furthest pair verification") {
    WorldPath poly;
    poly.addPoint({5.0, 0.0});
    poly.addPoint({0.0, 5.0});
    poly.addPoint({-5.0, 0.0});
    poly.addPoint({0.0, -5.0});
    // Furthest pairs are opposing vertices, distance 10
    auto [a, b] = poly.furthestPair();
    double d = std::hypot(b.getX() - a.getX(), b.getY() - a.getY());
    REQUIRE_THAT(d, Catch::Matchers::WithinRel(10.0));
  }
}

TEST_CASE("WorldPath hasFoldbacks", "[worldpath]") {
  WorldPath path;

  SECTION("no foldbacks") {
    path.addPoint({0.0, 0.0});
    path.addPoint({5.0, 0.0});
    path.addPoint({5.0, 5.0});
    REQUIRE_FALSE(path.hasFoldbacks());
  }

  SECTION("perfect foldback") {
    path.addPoint({0.0, 0.0});
    path.addPoint({5.0, 0.0});
    path.addPoint({2.0, 0.0});
    REQUIRE(path.hasFoldbacks());
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
    REQUIRE_THAT(a.back().getX(), Catch::Matchers::WithinRel(3.0));
  }

  SECTION("append to empty path") {
    WorldPath a;
    WorldPath b;
    b.addPoint({1.0, 1.0});
    b.addPoint({2.0, 2.0});

    a.append(b);
    REQUIRE(a.size() == 2);
    REQUIRE_THAT(a.front().getX(), Catch::Matchers::WithinRel(1.0));
  }

  SECTION("append path to itself") {
    WorldPath a;
    a.addPoint({1.0, 0.0});
    a.addPoint({2.0, 0.0});

    a.append(a);
    REQUIRE(a.size() == 4);
    REQUIRE_THAT(a[0].getX(), Catch::Matchers::WithinRel(1.0));
    REQUIRE_THAT(a[2].getX(), Catch::Matchers::WithinRel(1.0));
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
    REQUIRE_THAT(r.front().getX(), Catch::Matchers::WithinRel(7.0));
    REQUIRE_THAT(r.front().getY(), Catch::Matchers::WithinRel(-2.0));
  }

  SECTION("two-point path swaps start and end") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({5.0, 5.0});

    WorldPath r = path.reversed();
    REQUIRE_THAT(r.front().getX(), Catch::Matchers::WithinRel(5.0));
    REQUIRE_THAT(r.back().getX(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("multi-point path") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({2.0, 0.0});
    path.addPoint({3.0, 0.0});

    WorldPath r = path.reversed();
    REQUIRE_THAT(r.front().getX(), Catch::Matchers::WithinRel(3.0));
    REQUIRE_THAT(r.back().getX(), Catch::Matchers::WithinRel(0.0));
  }
}

TEST_CASE("WorldPath pointAtDistance", "[worldpath]") {
  WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  SECTION("clamps before start") {
    auto p = path.pointAtDistance(-1.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("returns exact start") {
    auto p = path.pointAtDistance(0.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("interpolates inside first segment") {
    auto p = path.pointAtDistance(2.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(2.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("hits exact vertex") {
    auto p = path.pointAtDistance(3.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(3.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("interpolates across second segment") {
    auto p = path.pointAtDistance(4.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(3.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(1.0));
  }

  SECTION("clamps after end") {
    auto p = path.pointAtDistance(100.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(3.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(4.0));
  }

  SECTION("handles zero-length segments") {
    WorldPath dup;
    dup.addPoint({0.0, 0.0});
    dup.addPoint({0.0, 0.0});
    dup.addPoint({5.0, 0.0});

    auto p = dup.pointAtDistance(2.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(2.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
  }

  SECTION("interpolation deep inside a complex path") {
    WorldPath long_path;
    long_path.addPoint({0.0, 0.0});
    for (int i = 1; i <= 20; ++i) {
      long_path.addPoint(
          {static_cast<double>(i * 10), 0.0});  // segments of length 10
    }
    // Deep inside segment 15
    // Distance to point 15 is 15 * 10 = 150.
    // 155 is exactly halfway between index 15 and 16 (X=150 and X=160).
    auto p = long_path.pointAtDistance(155.0);
    REQUIRE_THAT(p.getX(), Catch::Matchers::WithinRel(155.0));
    REQUIRE_THAT(p.getY(), Catch::Matchers::WithinRel(0.0));
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

  SECTION("longer clean path does not false-positive") {
    WorldPath path;
    path.addPoint({0.0, 0.0});
    path.addPoint({1.0, 0.0});
    path.addPoint({2.0, 1.0});
    path.addPoint({3.0, 0.0});
    path.addPoint({4.0, 0.0});
    path.addPoint({5.0, 1.0});
    REQUIRE_FALSE(path.selfIntersects());
  }

  SECTION("tight non-intersecting spiral (stress test sweep boundaries)") {
    WorldPath spiral;
    // Spiral inward without ever crossing
    spiral.addPoint({10.0, 10.0});
    spiral.addPoint({10.0, -10.0});
    spiral.addPoint({-10.0, -10.0});
    spiral.addPoint({-10.0, 8.0});
    spiral.addPoint({8.0, 8.0});
    spiral.addPoint({8.0, -8.0});
    spiral.addPoint({-8.0, -8.0});
    spiral.addPoint({-8.0, 6.0});
    spiral.addPoint({6.0, 6.0});
    spiral.addPoint({6.0, -6.0});
    spiral.addPoint({-6.0, -6.0});
    spiral.addPoint({-6.0, 4.0});

    REQUIRE_FALSE(spiral.selfIntersects());
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
