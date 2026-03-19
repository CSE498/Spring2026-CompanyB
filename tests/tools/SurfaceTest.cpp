// test_surface_catch2.cpp
// #include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <catch2/catch_test_macros.hpp>
#include <set>
#include <tuple>
#include <vector>

#include "../../source/tools/Box.hpp"
#include "../../source/tools/Circle.hpp"
#include "../../source/tools/Point.hpp"
#include "../../source/tools/Surface.hpp"

using namespace cse498;

TEST_CASE("Surface; add shapes and detect overlap", "surface circle") {
  Surface::Config cfg;
  cfg.sector_size = 5.0;
  Surface s(cfg);

  Circle c1(Point(0.0, 0.0), 1.0);
  Circle c2(Point(1.5, 0.0), 1.0);
  Circle c3(Point(10.0, 10.0), 1.0);

  auto id1 = s.AddCircle(c1);
  auto id2 = s.AddCircle(c2);
  auto id3 = s.AddCircle(c3);

  REQUIRE(id1 != id2);
  REQUIRE(id1 != id3);
  REQUIRE(id2 != id3);

  auto pairs = s.DetectAllOverlaps();

  auto contains_pair = [&](Surface::ShapeID a, Surface::ShapeID b) {
    for (const auto& p : pairs) {
      if ((p.first == a && p.second == b) || (p.first == b && p.second == a))
        return true;
    }
    return false;
  };

  REQUIRE(contains_pair(id1, id2));
  REQUIRE_FALSE(contains_pair(id1, id3));
  REQUIRE_FALSE(contains_pair(id2, id3));
}

TEST_CASE("Surface; add boxes and detect overlap", "surface box") {
  Surface::Config cfg;
  cfg.sector_size = 4.0;
  Surface s(cfg);

  Box b1 = Box::FromCorners(Point(-1.0, -1.0), Point(1.0, 1.0));
  Box b2 = Box::FromCorners(Point(0.5, 0.5), Point(2.0, 2.0));  // overlaps b1
  Box b3 = Box::FromCorners(Point(5.0, 5.0), Point(6.0, 6.0));  // far away

  auto idb1 = s.AddBox(b1);
  auto idb2 = s.AddBox(b2);
  auto idb3 = s.AddBox(b3);

  auto pairs = s.DetectAllOverlaps();
  auto contains_pair = [&](Surface::ShapeID a, Surface::ShapeID b) {
    for (const auto& p : pairs) {
      if ((p.first == a && p.second == b) || (p.first == b && p.second == a))
        return true;
    }
    return false;
  };

  REQUIRE(contains_pair(idb1, idb2));
  REQUIRE_FALSE(contains_pair(idb1, idb3));
  REQUIRE_FALSE(contains_pair(idb2, idb3));
}

TEST_CASE("overlap detection and QueryRadius", "surface mixed query") {
  Surface::Config cfg;
  cfg.sector_size = 5.0;
  Surface s(cfg);

  Circle c(Point(0.0, 0.0), 1.0);
  Box b = Box::FromCorners(Point(-0.5, -0.5), Point(0.5, 0.5));
  Box b_far = Box::FromCorners(Point(10.0, 10.0), Point(11.0, 11.0));

  auto idc = s.AddCircle(c);
  auto idb = s.AddBox(b);
  auto idbf = s.AddBox(b_far);

  auto pairs = s.DetectAllOverlaps();
  auto contains_pair = [&](Surface::ShapeID a, Surface::ShapeID b) {
    for (const auto& p : pairs) {
      if ((p.first == a && p.second == b) || (p.first == b && p.second == a))
        return true;
    }
    return false;
  };

  REQUIRE(contains_pair(idc, idb));
  REQUIRE_FALSE(contains_pair(idc, idbf));

  auto q = s.QueryRadius(Point(0.0, 0.0), 2.0);
  std::set<Surface::ShapeID> qset(q.begin(), q.end());
  REQUIRE(qset.count(idc) == 1);
  REQUIRE(qset.count(idb) == 1);
  REQUIRE(qset.count(idbf) == 0);
}

TEST_CASE("Surface: overlap callbacks ", "surface callbacks") {
  Surface::Config cfg;
  cfg.sector_size = 5.0;
  Surface s(cfg);

  Circle c1(Point(0.0, 0.0), 1.0);
  Circle c2(Point(1.5, 0.0), 1.0);

  auto id1 = s.AddCircle(c1);
  auto id2 = s.AddCircle(c2);

  std::vector<std::tuple<Surface::ShapeID, Surface::ShapeID, bool>> events;
  s.SetOverlapCallback(
      [&events](Surface::ShapeID a, Surface::ShapeID b, bool started) {
        events.emplace_back(a, b, started);
      });

  s.step();
  bool got_start = false;
  for (auto& ev : events) {
    auto a = std::get<0>(ev);
    auto b = std::get<1>(ev);
    auto sflag = std::get<2>(ev);
    if (sflag && ((a == id1 && b == id2) || (a == id2 && b == id1)))
      got_start = true;
  }
  REQUIRE(got_start);
  events.clear();

  bool ok = s.TranslateShape(id2, Point(10.0, 0.0));
  REQUIRE(ok);
  s.step();

  bool got_end = false;
  for (auto& ev : events) {
    auto a = std::get<0>(ev);
    auto b = std::get<1>(ev);
    auto sflag = std::get<2>(ev);
    if (!sflag && ((a == id1 && b == id2) || (a == id2 && b == id1)))
      got_end = true;
  }
  REQUIRE(got_end);
  events.clear();

  REQUIRE(s.TranslateShape(id2, Point(-10.0, 0.0)));
  s.step();
  bool got_restart = false;
  for (auto& ev : events) {
    auto a = std::get<0>(ev);
    auto b = std::get<1>(ev);
    auto sflag = std::get<2>(ev);
    if (sflag && ((a == id1 && b == id2) || (a == id2 && b == id1)))
      got_restart = true;
  }
  REQUIRE(got_restart);
}

TEST_CASE("Remove shape", "surface remove") {
  Surface::Config cfg;
  cfg.sector_size = 5.0;
  Surface s(cfg);

  Circle c(Point(0.0, 0.0), 1.0);
  Circle c2(Point(1.2, 0.0), 1.0);

  auto id1 = s.AddCircle(c);
  auto id2 = s.AddCircle(c2);

  auto pairs_before = s.DetectAllOverlaps();
  auto contains_pair = [&](Surface::ShapeID a, Surface::ShapeID b) {
    for (const auto& p : pairs_before) {
      if ((p.first == a && p.second == b) || (p.first == b && p.second == a))
        return true;
    }
    return false;
  };
  REQUIRE(contains_pair(id1, id2));

  REQUIRE(s.RemoveShape(id1));
  REQUIRE_FALSE(s.RemoveShape(id1));

  auto pairs_after = s.DetectAllOverlaps();
  bool still_overlapping = false;
  for (const auto& p : pairs_after) {
    if ((p.first == id1 && p.second == id2) ||
        (p.first == id2 && p.second == id1)) {
      still_overlapping = true;
      break;
    }
  }
  REQUIRE_FALSE(still_overlapping);

  auto ids = s.AllShapeIDs();
  REQUIRE(ids.size() == 1);
  REQUIRE(ids[0] == id2);
}
