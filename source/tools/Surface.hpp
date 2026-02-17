// Surface.hpp
#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <cmath>
#include <cassert>
#include <limits>
#include <variant>
#include <algorithm>

#include "Circle.h"
#include "Box.h"

/**
 * @brief Surface that stores Circle or Box shapes and detects overlaps.
 *
 * Shapes are stored as std::variant<Circle, Box>. All overlap checks are
 * implemented for the three combinations:
 *  - Circle vs Circle
 *  - Box vs Box
 *  - Circle vs Box
 *
 * Uses a uniform grid (square sectors) for partitioning.
 */
namespace cse498 {
class Surface {
public:
  using ShapeID = uint64_t;
  using OverlapCallback = std::function<void(ShapeID a, ShapeID b, bool started)>;

  struct Config {
    double sector_size = 10.0;
    Config() = default;
  };

  Surface(const Config& cfg = Config())
    : cfg_(cfg), next_id_(1) {
    assert(cfg_.sector_size > 0.0);
  }
  ~Surface() = default;
  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;


  struct Entry {
    std::variant<Circle, Box> shape;
    ShapeID id = 0;
    std::vector<uint64_t> occupied_cells;
  };


  ShapeID AddCircle(const Circle& c) {
    ShapeID id = next_id_++;
    Entry e;
    e.shape = c;
    e.id = id;
    entries_.emplace(id, std::move(e));
    InsertIntoSectors(id);
    return id;
  }

  ShapeID AddBox(const Box& b) {
    ShapeID id = next_id_++;
    Entry e;
    e.shape = b;
    e.id = id;
    entries_.emplace(id, std::move(e));
    InsertIntoSectors(id);
    return id;
  }

  bool RemoveShape(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    entries_.erase(it);
    RemoveOverlapsFor(id);
    return true;
  }

  bool UpdateCircle(ShapeID id, const Circle& c) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    it->second.shape = c;
    InsertIntoSectors(id);
    return true;
  }
  bool UpdateBox(ShapeID id, const Box& b) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    it->second.shape = b;
    InsertIntoSectors(id);
    return true;
  }

  bool TranslateShape(ShapeID id, const Point& delta) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    std::visit([&](auto &s) { s.Translate(delta); }, it->second.shape);
    InsertIntoSectors(id);
    return true;
  }

  std::vector<ShapeID> AllShapeIDs() const {
    std::vector<ShapeID> out;
    out.reserve(entries_.size());
    for (const auto& kv : entries_) out.push_back(kv.first);
    return out;
  }


  std::vector<ShapeID> QueryRadius(const Point& center, double radius) const {
    std::vector<ShapeID> out;
    if (entries_.empty()) return out;
    int minx = CellX(center.x() - radius);
    int maxx = CellX(center.x() + radius);
    int miny = CellY(center.y() - radius);
    int maxy = CellY(center.y() + radius);

    std::unordered_set<ShapeID> seen;
    for (int cx = minx; cx <= maxx; ++cx) {
      for (int cy = miny; cy <= maxy; ++cy) {
        uint64_t key = KeyFor(cx, cy);
        auto itr = sectors_.find(key);
        if (itr == sectors_.end()) continue;
        for (ShapeID id : itr->second) {
          if (seen.emplace(id).second) {
            const Entry& e = entries_.at(id);
            if (ShapeOverlapsCircle(e, center, radius)) {
              out.push_back(id);
            }
          }
        }
      }
    }
    return out;
  }

  std::vector<std::pair<ShapeID, ShapeID>> DetectAllOverlaps() const {
    std::vector<std::pair<ShapeID, ShapeID>> out;
    if (entries_.empty()) return out;

    PairSet found;

    // per-sector pair checks
    for (const auto& kv : sectors_) {
      const std::vector<ShapeID>& celllist = kv.second;
      const size_t n = celllist.size();
      for (size_t i = 0; i < n; ++i) {
        ShapeID a = celllist[i];
        auto itA = entries_.find(a);
        if (itA == entries_.end()) continue;
        for (size_t j = i + 1; j < n; ++j) {
          ShapeID b = celllist[j];
          auto itB = entries_.find(b);
          if (itB == entries_.end()) continue;
          ShapePair sp = MakeOrderedPair(a, b);
          if (found.find(sp) != found.end()) continue;
          if (ShapesOverlap(itA->second, itB->second)) {
            found.insert(sp);
            out.emplace_back(sp.first, sp.second);
          }
        }
      }
    }


    for (const auto& kv : entries_) {
      ShapeID a = kv.first;
      const Entry& ea = kv.second;
      auto [aminx, aminy, amaxx, amaxy] = BoundingCellRange(ea);
      std::unordered_set<ShapeID> seen;
      for (int cx = aminx; cx <= amaxx; ++cx) {
        for (int cy = aminy; cy <= amaxy; ++cy) {
          auto itSec = sectors_.find(KeyFor(cx, cy));
          if (itSec == sectors_.end()) continue;
          for (ShapeID b : itSec->second) {
            if (b == a) continue;
            if (!seen.insert(b).second) continue;
            ShapePair sp = MakeOrderedPair(a, b);
            if (found.find(sp) != found.end()) continue;
            auto itB = entries_.find(b);
            if (itB == entries_.end()) continue;
            if (ShapesOverlap(ea, itB->second)) {
              found.insert(sp);
              out.emplace_back(sp.first, sp.second);
            }
          }
        }
      }
    }

    return out;
  }

  void SetOverlapCallback(OverlapCallback cb) { overlap_cb_ = std::move(cb); }

  void step() {
    auto current_pairs = DetectAllOverlaps();
    PairSet currentSet;
    for (const auto& p : current_pairs) currentSet.insert(MakeOrderedPair(p.first, p.second));

    for (const auto& sp : currentSet) {
      if (prev_overlaps_.find(sp) == prev_overlaps_.end()) {
        if (overlap_cb_) overlap_cb_(sp.first, sp.second, true);
      }
    }
    for (const auto& sp : prev_overlaps_) {
      if (currentSet.find(sp) == currentSet.end()) {
        if (overlap_cb_) overlap_cb_(sp.first, sp.second, false);
      }
    }
    prev_overlaps_.swap(currentSet);
  }

  void clear() {
    entries_.clear();
    sectors_.clear();
    prev_overlaps_.clear();
  }

private:
  Config cfg_;
  ShapeID next_id_;
  std::unordered_map<ShapeID, Entry> entries_;
  std::unordered_map<uint64_t, std::vector<ShapeID>> sectors_;
  OverlapCallback overlap_cb_;

  struct ShapePair {
    ShapeID first;
    ShapeID second;
    bool operator==(const ShapePair& o) const noexcept {
      return first == o.first && second == o.second;
    }
  };
  struct PairHash {
    size_t operator()(const ShapePair& p) const noexcept {
      uint64_t a = p.first;
      uint64_t b = p.second;
      uint64_t x = a * 0x9ddfea08eb382d69ULL + (b ^ 0xc3a5c85c97cb3127ULL);
      return static_cast<size_t>(x ^ (x >> 32));
    }
  };
  struct PairEq {
    bool operator()(const ShapePair& a, const ShapePair& b) const noexcept {
      return a.first == b.first && a.second == b.second;
    }
  };
  using PairSet = std::unordered_set<ShapePair, PairHash, PairEq>;
  PairSet prev_overlaps_;

  inline int CellX(double x) const noexcept { return static_cast<int>(std::floor(x / cfg_.sector_size)); }
  inline int CellY(double y) const noexcept { return static_cast<int>(std::floor(y / cfg_.sector_size)); }
  static uint64_t KeyForXYInts(int x, int y) noexcept {
    uint64_t ux = static_cast<uint32_t>(x);
    uint64_t uy = static_cast<uint32_t>(y);
    return (ux << 32) | uy;
  }
  inline uint64_t KeyFor(int cx, int cy) const noexcept { return KeyForXYInts(cx, cy); }
  static ShapePair MakeOrderedPair(ShapeID a, ShapeID b) noexcept { return a < b ? ShapePair{a,b} : ShapePair{b,a}; }


  std::tuple<int,int,int,int> BoundingCellRange(const Entry& e) const {
    double minx_d, miny_d, maxx_d, maxy_d;
    if (std::holds_alternative<Circle>(e.shape)) {
      const Circle& c = std::get<Circle>(e.shape);
      const Point& center = c.GetCenter();
      double r = c.GetRadius();
      minx_d = center.x() - r;
      maxx_d = center.x() + r;
      miny_d = center.y() - r;
      maxy_d = center.y() + r;
    } else {
      const Box& b = std::get<Box>(e.shape);
      minx_d = b.GetMin().x();
      miny_d = b.GetMin().y();
      maxx_d = b.GetMax().x();
      maxy_d = b.GetMax().y();
    }
    int minx = CellX(minx_d);
    int maxx = CellX(maxx_d);
    int miny = CellY(miny_d);
    int maxy = CellY(maxy_d);
    return {minx, miny, maxx, maxy};
  }


  void InsertIntoSectors(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return;
    Entry& e = it->second;
    auto [minx, miny, maxx, maxy] = BoundingCellRange(e);
    e.occupied_cells.clear();
    for (int cx = minx; cx <= maxx; ++cx) {
      for (int cy = miny; cy <= maxy; ++cy) {
        uint64_t key = KeyFor(cx, cy);
        auto &vec = sectors_[key];
        vec.push_back(id);
        e.occupied_cells.push_back(key);
      }
    }
  }

  void RemoveFromSectors(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return;
    Entry& e = it->second;
    for (uint64_t key : e.occupied_cells) {
      auto sit = sectors_.find(key);
      if (sit == sectors_.end()) continue;
      auto &vec = sit->second;
      auto itv = std::find(vec.begin(), vec.end(), id);
      if (itv != vec.end()) {
        *itv = vec.back();
        vec.pop_back();
      }
      if (vec.empty()) sectors_.erase(key);
    }
    e.occupied_cells.clear();
  }

  void RemoveOverlapsFor(ShapeID id) {
    std::vector<ShapePair> to_remove;
    for (const auto &p : prev_overlaps_) {
      if (p.first == id || p.second == id) to_remove.push_back(p);
    }
    for (const auto &p : to_remove) prev_overlaps_.erase(p);
  }


  static bool ShapesOverlap(const Entry& a, const Entry& b) {
    const auto& va = a.shape;
    const auto& vb = b.shape;
    if (std::holds_alternative<Circle>(va) && std::holds_alternative<Circle>(vb)) {
      const Circle& ca = std::get<Circle>(va);
      const Circle& cb = std::get<Circle>(vb);
      return ca.Overlaps(cb);
    }
    if (std::holds_alternative<Box>(va) && std::holds_alternative<Box>(vb)) {
      const Box& ba = std::get<Box>(va);
      const Box& bb = std::get<Box>(vb);
      return ba.Intersects(bb);
    }
    // one circle, one box:
    if (std::holds_alternative<Circle>(va) && std::holds_alternative<Box>(vb)) {
      const Circle& ca = std::get<Circle>(va);
      const Box& bb = std::get<Box>(vb);
      return bb.Intersects(ca);
    }
    if (std::holds_alternative<Box>(va) && std::holds_alternative<Circle>(vb)) {
      const Box& ba = std::get<Box>(va);
      const Circle& cb = std::get<Circle>(vb);
      return ba.Intersects(cb);
    }
    return false;
  }


  static bool ShapeOverlapsCircle(const Entry& e, const Point& center, double radius) {
    if (std::holds_alternative<Circle>(e.shape)) {
      const Circle& c = std::get<Circle>(e.shape);
      double d = c.DistanceTo(center);
      return d <= (c.GetRadius() + radius + Circle::kEps);
    } else {
      const Box& b = std::get<Box>(e.shape);
      // nearest point on AABB to center
      double cx = center.x();
      double cy = center.y();
      double closestX = std::clamp(cx, b.GetMin().x(), b.GetMax().x());
      double closestY = std::clamp(cy, b.GetMin().y(), b.GetMax().y());
      double dx = cx - closestX;
      double dy = cy - closestY;
      return (dx*dx + dy*dy) <= (radius + Circle::kEps) * (radius + Circle::kEps);
    }
  }
};
}