/// @file WorldPath.hpp
/// @author Devansh Tayal
/// @brief A list of ordered points representing an agent's path.

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Math/Point.hpp"

namespace cse498 {

/**
 * @brief Ordered list of 2-D waypoints that an agent can follow.
 *
 * Includes helpers for measuring length, checking self-intersection,
 * interpolating along the path, etc.
 */
class WorldPath {
public:
  /// Tolerance for floating-point comparisons in geometry helpers.
  static constexpr double kDefaultEps = 1e-9;

  auto begin() { return points_.begin(); }
  auto end() { return points_.end(); }
  auto begin() const { return points_.begin(); }
  auto end() const { return points_.end(); }

  Point &operator[](std::size_t i) { return points_[i]; }
  const Point &operator[](std::size_t i) const { return points_[i]; }

  /**
   * @brief Bounds-checked access to the i-th point.
   * @param i Index of the point.
   * @return Reference to the point at index i.
   * @throws std::out_of_range if i >= size().
   */
  Point &at(std::size_t i) {
    if (i >= points_.size())
      throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  /** @copydoc at(std::size_t) */
  const Point &at(std::size_t i) const {
    if (i >= points_.size())
      throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  Point &front() {
    assert(!points_.empty());
    return points_.front();
  }
  const Point &front() const {
    assert(!points_.empty());
    return points_.front();
  }
  Point &back() {
    assert(!points_.empty());
    return points_.back();
  }
  const Point &back() const {
    assert(!points_.empty());
    return points_.back();
  }

  [[nodiscard]] bool empty() const { return points_.empty(); }
  std::size_t size() const { return points_.size(); }
  void reserve(std::size_t n) { points_.reserve(n); }
  void clear() { points_.clear(); }

  /**
   * @brief Appends a point to the path.
   * @param p Point to add. Must have finite coordinates (asserts on NaN/Inf).
   */
  void addPoint(const Point &p) {
    assert(isValidPoint(p));
    points_.push_back(p);
  }

  /**
   * @brief Removes the last point.
   * @return false if the path was already empty, true otherwise.
   */
  [[nodiscard]] bool popBack() {
    if (points_.empty())
      return false;
    points_.pop_back();
    return true;
  }

  /**
   * @brief Read-only span over the internal point storage.
   * @return A std::span of const Points.
   */
  std::span<const Point> pointsView() const { return points_; }

  /**
   * @brief Consecutive point pairs along the path.
   * @return A pairwise view of the points.
   */
  auto segments() const { return points_ | std::views::pairwise; }

  /**
   * @brief Checks that every point has finite coordinates.
   * @return true if all points are finite, false otherwise.
   */
  bool isValid() const { return std::ranges::all_of(points_, isValidPoint); }

  /**
   * @brief Total path length (sum of segment distances).
   * @return 0.0 for empty or single-point paths.
   */
  double totalLength() const {
    return std::ranges::fold_left(
        segments(), 0.0, [](double acc, const auto &seg) {
          return acc + dist(std::get<0>(seg), std::get<1>(seg));
        });
  }

  /**
   * @brief Length of segment i (between point i and point i+1).
   * @param i Segment index.
   * @return The segment length, or nullopt if i is out of range.
   */
  std::optional<double> segmentLength(std::size_t i) const {
    if (i + 1 >= points_.size())
      return std::nullopt;
    return dist(points_[i], points_[i + 1]);
  }

  /**
   * @brief Brute-force O(n^2) search for the two most distant points.
   * @return The two points with the greatest Euclidean distance.
   */
  std::pair<Point, Point> furthestPair() const {
    assert(points_.size() >= 2);
    std::size_t ai = 0;
    std::size_t bi = 1;
    double best = dist(points_[ai], points_[bi]);
    for (std::size_t i = 0; i < points_.size(); ++i) {
      for (std::size_t j = i + 1; j < points_.size(); ++j) {
        double d = dist(points_[i], points_[j]);
        if (d > best) {
          best = d;
          ai = i;
          bi = j;
        }
      }
    }
    return {points_[ai], points_[bi]};
  }

  /**
   * @brief Checks if the path forms a closed loop.
   * @param eps Tolerance for comparing the first and last points.
   * @return true if start and end points are the same within eps.
   */
  bool isClosed(double eps = kDefaultEps) const {
    return points_.size() >= 2 &&
           samePoint(points_.front(), points_.back(), eps);
  }

  /**
   * @brief Tacks another path's points onto the end of this one.
   * @param other Path whose points will be appended.
   */
  void append(const WorldPath &other) {
    points_.insert(points_.end(), other.points_.begin(), other.points_.end());
  }

  /**
   * @brief Returns a copy with points in reverse order.
   * @return A new WorldPath with reversed point order.
   */
  WorldPath reversed() const {
    WorldPath rev;
    rev.points_.assign(points_.rbegin(), points_.rend());
    return rev;
  }

  /**
   * @brief Get the interpolated point at a given distance along the path.
   * @param target Distance along the path to sample at.
   * @return The interpolated point. Clamps to first/last point if out of range.
   */
  Point pointAtDistance(double target) const {
    assert(!points_.empty());
    if (target <= 0.0)
      return points_.front();

    double traveled = 0.0;
    for (auto [a, b] : segments()) {
      double len = dist(a, b);
      if (len > 0.0 && traveled + len >= target) {
        double t = (target - traveled) / len;
        return {a.x() + t * (b.x() - a.x()), a.y() + t * (b.y() - a.y())};
      }
      traveled += len;
    }
    return points_.back();
  }

  /**
   * @brief O(n^2) check for any two non-adjacent segments crossing.
   *
   * Could be improved with a sweep-line algorithm for large paths.
   *
   * @return true if any non-adjacent segments intersect.
   */
  bool selfIntersects() const {
    const std::size_t n = points_.size();
    if (n < 4)
      return false;
    bool closed = samePoint(points_.front(), points_.back(), kDefaultEps);
    for (std::size_t i = 0; i + 1 < n; ++i) {
      for (std::size_t j = i + 2; j + 1 < n; ++j) {
        // Skip the first-last segment pair for closed paths (they share a
        // vertex).
        if (closed && i == 0 && j == n - 2)
          continue;
        if (segmentsIntersect(points_[i], points_[i + 1], points_[j],
                              points_[j + 1], kDefaultEps))
          return true;
      }
    }
    return false;
  }

private:
  std::vector<Point> points_;

  // Geometry helpers (replace with Group 13's utilities when available)

  static bool isValidPoint(const Point &p) {
    return std::isfinite(p.x()) && std::isfinite(p.y());
  }

  static double dist(const Point &a, const Point &b) {
    double dx = b.x() - a.x();
    double dy = b.y() - a.y();
    return std::sqrt(dx * dx + dy * dy);
  }

  static bool nearlyEq(double a, double b, double eps = kDefaultEps) {
    return std::abs(a - b) <= eps;
  }

  static bool samePoint(const Point &a, const Point &b,
                        double eps = kDefaultEps) {
    return nearlyEq(a.x(), b.x(), eps) && nearlyEq(a.y(), b.y(), eps);
  }

  static int orient(const Point &a, const Point &b, const Point &c,
                    double eps = kDefaultEps) {
    double cross =
        (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
    if (nearlyEq(cross, 0.0, eps))
      return 0;
    return (cross > 0.0) ? 1 : -1;
  }

  static bool onSegment(const Point &seg_start, const Point &query,
                        const Point &seg_end, double eps = kDefaultEps) {
    return query.x() >= std::min(seg_start.x(), seg_end.x()) - eps &&
           query.x() <= std::max(seg_start.x(), seg_end.x()) + eps &&
           query.y() >= std::min(seg_start.y(), seg_end.y()) - eps &&
           query.y() <= std::max(seg_start.y(), seg_end.y()) + eps;
  }

  static bool segmentsIntersect(const Point &a1, const Point &a2,
                                const Point &b1, const Point &b2,
                                double eps = kDefaultEps) {
    int d1 = orient(a1, a2, b1, eps);
    int d2 = orient(a1, a2, b2, eps);
    int d3 = orient(b1, b2, a1, eps);
    int d4 = orient(b1, b2, a2, eps);
    if (d1 != d2 && d3 != d4)
      return true;
    if (d1 == 0 && onSegment(a1, b1, a2, eps))
      return true;
    if (d2 == 0 && onSegment(a1, b2, a2, eps))
      return true;
    if (d3 == 0 && onSegment(b1, a1, b2, eps))
      return true;
    if (d4 == 0 && onSegment(b1, a2, b2, eps))
      return true;
    return false;
  }
};

} // namespace cse498
