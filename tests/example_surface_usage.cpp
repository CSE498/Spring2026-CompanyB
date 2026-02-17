// test_surface.cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <set>

#include "Surface.h"
#include "Circle.h"
#include "Box.h"
#include "Point.h"

int main() {
  Surface::Config cfg;
  cfg.sector_size = 5.0;
  Surface surface(cfg);

  Circle c1(Point(0.0, 0.0), 1.0);
  Circle c2(Point(1.5, 0.0), 1.0);  // overlaps c1
  Circle c3(Point(10.0, 10.0), 1.0); // far away

  auto id1 = surface.AddCircle(c1);
  auto id2 = surface.AddCircle(c2);
  auto id3 = surface.AddCircle(c3);


  Box b1(Point(-0.5, -0.5), Point(0.5, 0.5));
  auto idb1 = surface.AddBox(b1);


  Box b2(Point(0.4, 0.4), Point(2.0, 2.0));
  auto idb2 = surface.AddBox(b2);


  auto pairs = surface.DetectAllOverlaps();
  // Expect c1 & c2 overlap, c1 & b1 overlap, b1 & b2 may overlap, etc.
  auto contains_pair = [&](Surface::ShapeID a, Surface::ShapeID b) {
    for (auto &p : pairs) {
      if ((p.first == a && p.second == b) || (p.first == b && p.second == a)) return true;
    }
    return false;
  };

  assert(contains_pair(id1, id2) && "Expected circle c1 and c2 to overlap");
  assert(contains_pair(id1, idb1) && "Expected circle c1 and box b1 to overlap");
  assert(contains_pair(idb1, idb2) && "Expected box b1 and box b2 to overlap");


  for (auto &p : pairs) {
    assert(!(p.first == id3 || p.second == id3));
  }


  auto q = surface.QueryRadius(Point(0.0, 0.0), 2.0);
  std::set<Surface::ShapeID> qset(q.begin(), q.end());
  assert(qset.count(id1));
  assert(qset.count(id2));
  assert(qset.count(idb1));
  assert(qset.count(id3) == 0);


  std::vector<std::tuple<Surface::ShapeID, Surface::ShapeID, bool>> events;
  surface.SetOverlapCallback([&events](Surface::ShapeID a, Surface::ShapeID b, bool started) {
    events.emplace_back(a,b,started);
  });

  surface.step();

  bool start_c1_b1 = false, start_c1_c2 = false, start_b1_b2 = false;
  for (auto &ev : events) {
    auto a = std::get<0>(ev);
    auto b = std::get<1>(ev);
    auto s = std::get<2>(ev);
    if (s && ((a == id1 && b == idb1) || (a == idb1 && b == id1))) start_c1_b1 = true;
    if (s && ((a == id1 && b == id2) || (a == id2 && b == id1))) start_c1_c2 = true;
    if (s && ((a == idb1 && b == idb2) || (a == idb2 && b == idb1))) start_b1_b2 = true;
  }
  assert(start_c1_b1 && start_c1_c2 && start_b1_b2);
  events.clear();


  surface.TranslateShape(id2, Point(10.0, 0.0));
  surface.step();
  bool ended_c1_c2 = false;
  for (auto &ev : events) {
    if (!std::get<2>(ev) && ((std::get<0>(ev) == id1 && std::get<1>(ev) == id2) ||
                             (std::get<0>(ev) == id2 && std::get<1>(ev) == id1))) {
      ended_c1_c2 = true;
    }
  }
  assert(ended_c1_c2);
  events.clear();


  surface.TranslateShape(id2, Point(-10.0, 0.0));
  surface.step();
  bool restarted_c1_c2 = false;
  for (auto &ev : events) {
    if (std::get<2>(ev) && ((std::get<0>(ev) == id1 && std::get<1>(ev) == id2) ||
                             (std::get<0>(ev) == id2 && std::get<1>(ev) == id1))) {
      restarted_c1_c2 = true;
    }
  }
  assert(restarted_c1_c2);
  events.clear();


  surface.RemoveShape(id1);
  auto ids = surface.AllShapeIDs();
  for (auto id : ids) assert(id != id1);

  std::cout << "All Surface tests passed.\n";
  return 0;
}
