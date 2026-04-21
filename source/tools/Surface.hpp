/**
 * @file Surface.hpp
 * @brief Spatial registry for circles and boxes with overlap detection.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "Box.hpp"
#include "Circle.hpp"

namespace cse498 {

/**
 * @brief Surface that stores Circle and Box shapes and detects overlaps.
 *
 * Shapes are stored as std::variant<Circle, Box>. Surface uses a uniform grid
 * of square sectors as a broad-phase index before running exact overlap checks
 * for Circle/Circle, Box/Box, and Circle/Box pairs.
 */
class Surface {
 public:
  /// Stable ID assigned to each registered shape.
  using ShapeID = uint64_t;

  /**
   * @brief Callback type used for overlap start/end events.
   *
   * The bool parameter is true when an overlap starts and false when it ends.
   */
  using OverlapCallback =
      std::function<void(ShapeID a, ShapeID b, bool started)>;

  /**
   * @brief Surface configuration.
   */
  struct Config {
    /// Width and height of each square spatial sector.
    double sector_size = 10.0;
  };

  /**
   * @brief Construct a Surface with default configuration.
   */
  Surface() : Surface(Config{}) {}

  /**
   * @brief Construct a Surface with custom configuration.
   * @param cfg Configuration values. sector_size must be positive.
   */
  explicit Surface(const Config& cfg) : cfg_(cfg), next_id_(1) {
    assert(cfg_.sector_size > 0.0);
  }

  /**
   * @brief Default destructor.
   */
  ~Surface() = default;

  /// Surface owns shape IDs and sector state, so copies are disabled.
  Surface(const Surface&) = delete;

  /// Surface owns shape IDs and sector state, so copies are disabled.
  Surface& operator=(const Surface&) = delete;

  /**
   * @brief Stored shape entry.
   */
  struct Entry {
    /// Shape payload.
    std::variant<Circle, Box> shape;

    /// Stable shape ID.
    ShapeID id = 0;

    /// Sector keys currently occupied by this shape.
    std::vector<uint64_t> occupied_cells;
  };

  /**
   * @brief Add a circle shape.
   * @param circle Circle to store.
   * @return Newly assigned shape ID.
   */
  ShapeID AddCircle(const Circle& circle) {
    ShapeID id = next_id_++;
    Entry entry;
    entry.shape = circle;
    entry.id = id;
    entries_.emplace(id, std::move(entry));
    InsertIntoSectors(id);
    return id;
  }

  /**
   * @brief Add a box shape.
   * @param b Box to store.
   * @return Newly assigned shape ID.
   */
  ShapeID AddBox(const Box& b) {
    ShapeID id = next_id_++;
    Entry entry;
    entry.shape = b;
    entry.id = id;
    entries_.emplace(id, std::move(entry));
    InsertIntoSectors(id);
    return id;
  }

  /**
   * @brief Remove a shape by ID.
   * @param id Shape ID to remove.
   * @return true if a shape was removed; false if id was unknown.
   */
  bool RemoveShape(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    entries_.erase(it);
    RemoveOverlapsFor(id);
    return true;
  }

  /**
   * @brief Replace an existing circle shape.
   * @param id Shape ID to update.
   * @param circle Replacement circle.
   * @return true if the shape exists and was updated.
   */
  bool UpdateCircle(ShapeID id, const Circle& circle) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    it->second.shape = circle;
    InsertIntoSectors(id);
    return true;
  }

  /**
   * @brief Replace an existing box shape.
   * @param id Shape ID to update.
   * @param b Replacement box.
   * @return true if the shape exists and was updated.
   */
  bool UpdateBox(ShapeID id, const Box& b) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    it->second.shape = b;
    InsertIntoSectors(id);
    return true;
  }

  /**
   * @brief Translate a registered shape.
   * @param id Shape ID to translate.
   * @param delta Translation vector.
   * @return true if the shape exists and was translated.
   */
  bool TranslateShape(ShapeID id, const Point& delta) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return false;
    RemoveFromSectors(id);
    std::visit([&](auto& s) { s.Translate(delta); }, it->second.shape);
    InsertIntoSectors(id);
    return true;
  }

  /**
   * @brief Return IDs for all registered shapes.
   * @return Vector of shape IDs in unspecified order.
   */
  std::vector<ShapeID> AllShapeIDs() const {
    std::vector<ShapeID> out;
    out.reserve(entries_.size());
    for (const auto& kv : entries_) out.push_back(kv.first);
    return out;
  }

  /**
   * @brief Query shapes overlapping a circle.
   * @param center Query circle center.
   * @param radius Query circle radius.
   * @return IDs of shapes overlapping the query circle.
   */
  std::vector<ShapeID> QueryRadius(const Point& center, double radius) const {
    std::vector<ShapeID> out;
    if (entries_.empty()) return out;
    int minx = CellX(center.getX() - radius);
    int maxx = CellX(center.getX() + radius);
    int miny = CellY(center.getY() - radius);
    int maxy = CellY(center.getY() + radius);

    std::unordered_set<ShapeID> seen;
    for (int cx = minx; cx <= maxx; ++cx) {
      for (int cy = miny; cy <= maxy; ++cy) {
        uint64_t key = KeyFor(cx, cy);
        auto itr = sectors_.find(key);
        if (itr == sectors_.end()) continue;
        for (ShapeID id : itr->second) {
          if (seen.emplace(id).second) {
            const Entry& entry = entries_.at(id);
            if (ShapeOverlapsCircle(entry, center, radius)) {
              out.push_back(id);
            }
          }
        }
      }
    }
    return out;
  }

  /**
   * @brief Detect all currently overlapping shape pairs.
   * @return Ordered pairs of shape IDs where first < second.
   */
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

  /**
   * @brief Register an overlap transition callback.
   * @param cb Callback invoked by step(); pass empty callback to clear it.
   */
  void SetOverlapCallback(OverlapCallback cb) { overlap_cb_ = std::move(cb); }

  /**
   * @brief Emit overlap start/end events since the previous step.
   */
  void step() {
    auto current_pairs = DetectAllOverlaps();
    PairSet currentSet;
    for (const auto& p : current_pairs)
      currentSet.insert(MakeOrderedPair(p.first, p.second));

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

  /**
   * @brief Remove all shapes, sectors, and remembered overlap state.
   */
  void clear() {
    entries_.clear();
    sectors_.clear();
    prev_overlaps_.clear();
  }

 private:
  /// Active configuration.
  Config cfg_;

  /// Next shape ID to assign.
  ShapeID next_id_;

  /// Shape entries keyed by ShapeID.
  std::unordered_map<ShapeID, Entry> entries_;

  /// Spatial sector map from packed cell coordinates to shape IDs.
  std::unordered_map<uint64_t, std::vector<ShapeID>> sectors_;

  /// Optional overlap transition callback.
  OverlapCallback overlap_cb_;

  /// Ordered pair key used for deduplicating overlap pairs.
  struct ShapePair {
    ShapeID first;
    ShapeID second;
    bool operator==(const ShapePair& o) const noexcept {
      return first == o.first && second == o.second;
    }
  };

  /// Hash function for ShapePair.
  struct PairHash {
    size_t operator()(const ShapePair& p) const noexcept {
      uint64_t a = p.first;
      uint64_t b = p.second;
      uint64_t x = a * 0x9ddfea08eb382d69ULL + (b ^ 0xc3a5c85c97cb3127ULL);
      return static_cast<size_t>(x ^ (x >> 32));
    }
  };

  /// Equality predicate for ShapePair.
  struct PairEq {
    bool operator()(const ShapePair& a, const ShapePair& b) const noexcept {
      return a.first == b.first && a.second == b.second;
    }
  };

  /// Set type for overlap pair tracking.
  using PairSet = std::unordered_set<ShapePair, PairHash, PairEq>;

  /// Overlaps found during the previous step().
  PairSet prev_overlaps_;

  /**
   * @brief Convert an x coordinate into a sector coordinate.
   * @param x World x coordinate.
   * @return Sector x coordinate.
   */
  inline int CellX(double x) const noexcept {
    return static_cast<int>(std::floor(x / cfg_.sector_size));
  }

  /**
   * @brief Convert a y coordinate into a sector coordinate.
   * @param y World y coordinate.
   * @return Sector y coordinate.
   */
  inline int CellY(double y) const noexcept {
    return static_cast<int>(std::floor(y / cfg_.sector_size));
  }

  /**
   * @brief Pack integer sector coordinates into a uint64 key.
   * @param x Sector x coordinate.
   * @param y Sector y coordinate.
   * @return Packed key.
   */
  static uint64_t KeyForXYInts(int x, int y) noexcept {
    uint64_t ux = static_cast<uint32_t>(x);
    uint64_t uy = static_cast<uint32_t>(y);
    return (ux << 32) | uy;
  }

  /**
   * @brief Pack sector coordinates into a key.
   * @param cx Sector x coordinate.
   * @param cy Sector y coordinate.
   * @return Packed key.
   */
  inline uint64_t KeyFor(int cx, int cy) const noexcept {
    return KeyForXYInts(cx, cy);
  }

  /**
   * @brief Normalize a shape ID pair so first < second.
   * @param a First ID.
   * @param b Second ID.
   * @return Ordered pair.
   */
  static ShapePair MakeOrderedPair(ShapeID a, ShapeID b) noexcept {
    return a < b ? ShapePair{a, b} : ShapePair{b, a};
  }

  /**
   * @brief Compute the occupied sector bounds for a shape.
   * @param e Shape entry.
   * @return min x, min y, max x, max y sector coordinates.
   */
  std::tuple<int, int, int, int> BoundingCellRange(const Entry& e) const {
    double minx_d, miny_d, maxx_d, maxy_d;
    if (std::holds_alternative<Circle>(e.shape)) {
      const Circle& c = std::get<Circle>(e.shape);
      const Point& center = c.GetCenter();
      double r = c.GetRadius();
      minx_d = center.getX() - r;
      maxx_d = center.getX() + r;
      miny_d = center.getY() - r;
      maxy_d = center.getY() + r;
    } else {
      const Box& b = std::get<Box>(e.shape);
      minx_d = b.GetBottomLeft().getX();
      miny_d = b.GetBottomLeft().getY();
      maxx_d = b.GetTopRight().getX();
      maxy_d = b.GetTopRight().getY();
    }
    int minx = CellX(minx_d);
    int maxx = CellX(maxx_d);
    int miny = CellY(miny_d);
    int maxy = CellY(maxy_d);
    return {minx, miny, maxx, maxy};
  }

  /**
   * @brief Insert a shape into every sector its bounds touch.
   * @param id Shape ID to insert.
   */
  void InsertIntoSectors(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return;
    Entry& e = it->second;
    auto [minx, miny, maxx, maxy] = BoundingCellRange(e);
    e.occupied_cells.clear();
    for (int cx = minx; cx <= maxx; ++cx) {
      for (int cy = miny; cy <= maxy; ++cy) {
        uint64_t key = KeyFor(cx, cy);
        auto& vec = sectors_[key];
        vec.push_back(id);
        e.occupied_cells.push_back(key);
      }
    }
  }

  /**
   * @brief Remove a shape from all sectors it currently occupies.
   * @param id Shape ID to remove.
   */
  void RemoveFromSectors(ShapeID id) {
    auto it = entries_.find(id);
    if (it == entries_.end()) return;
    Entry& e = it->second;
    for (uint64_t key : e.occupied_cells) {
      auto sit = sectors_.find(key);
      if (sit == sectors_.end()) continue;
      auto& vec = sit->second;
      auto itv = std::find(vec.begin(), vec.end(), id);
      if (itv != vec.end()) {
        *itv = vec.back();
        vec.pop_back();
      }
      if (vec.empty()) sectors_.erase(key);
    }
    e.occupied_cells.clear();
  }

  /**
   * @brief Remove remembered overlap pairs involving a shape.
   * @param id Shape ID being removed.
   */
  void RemoveOverlapsFor(ShapeID id) {
    std::vector<ShapePair> to_remove;
    for (const auto& p : prev_overlaps_) {
      if (p.first == id || p.second == id) to_remove.push_back(p);
    }
    for (const auto& p : to_remove) prev_overlaps_.erase(p);
  }

  /**
   * @brief Run exact overlap check for two shape entries.
   * @param a First entry.
   * @param b Second entry.
   * @return true if the shapes overlap.
   */
  static bool ShapesOverlap(const Entry& a, const Entry& b) {
    const auto& va = a.shape;
    const auto& vb = b.shape;
    if (std::holds_alternative<Circle>(va) &&
        std::holds_alternative<Circle>(vb)) {
      const Circle& ca = std::get<Circle>(va);
      const Circle& cb = std::get<Circle>(vb);
      return ca.Overlaps(cb);
    }
    if (std::holds_alternative<Box>(va) && std::holds_alternative<Box>(vb)) {
      const Box& ba = std::get<Box>(va);
      const Box& bb = std::get<Box>(vb);
      return ba.Overlaps(bb);
    }
    // one circle, one box:
    if (std::holds_alternative<Circle>(va) && std::holds_alternative<Box>(vb)) {
      const Circle& ca = std::get<Circle>(va);
      const Box& bb = std::get<Box>(vb);
      return bb.Overlaps(ca);
    }
    if (std::holds_alternative<Box>(va) && std::holds_alternative<Circle>(vb)) {
      const Box& ba = std::get<Box>(va);
      const Circle& cb = std::get<Circle>(vb);
      return ba.Overlaps(cb);
    }
    return false;
  }

  /**
   * @brief Check whether an entry overlaps a query circle.
   * @param e Shape entry.
   * @param center Query center.
   * @param radius Query radius.
   * @return true if the entry overlaps the query circle.
   */
  static bool ShapeOverlapsCircle(const Entry& e, const Point& center,
                                  double radius) {
    if (std::holds_alternative<Circle>(e.shape)) {
      const Circle& c = std::get<Circle>(e.shape);
      double d = c.DistanceTo(center);
      return d <= (c.GetRadius() + radius + Circle::EPS);
    } else {
      const Box& b = std::get<Box>(e.shape);
      // nearest point on AABB to center
      double cx = center.getX();
      double cy = center.getY();
      double closestX =
          std::clamp(cx, b.GetBottomLeft().getX(), b.GetTopRight().getX());
      double closestY =
          std::clamp(cy, b.GetBottomLeft().getY(), b.GetTopRight().getY());
      double dx = cx - closestX;
      double dy = cy - closestY;
      return (dx * dx + dy * dy) <=
             (radius + Circle::EPS) * (radius + Circle::EPS);
    }
  }
};
}  // namespace cse498
