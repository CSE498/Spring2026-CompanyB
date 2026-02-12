// WorldPath is a list of ordered points representing an agent's path.
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

using Point = Math::Point;

class WorldPath {
 private:
  std::vector<Point> points;

  // TODO: replace with Point utilities once Group 13 provides them
  static bool isValidPoint(const Point& p) {
    return std::isfinite(p.x) && std::isfinite(p.y);
  }

  static double dist(const Point& a, const Point& b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
  }

  static bool nearlyEq(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) <= eps;
  }

  static bool samePoint(const Point& a, const Point& b, double eps = 1e-9) {
    return nearlyEq(a.x, b.x, eps) && nearlyEq(a.y, b.y, eps);
  }

  static int orient(const Point& a,
                    const Point& b,
                    const Point& c,
                    double eps = 1e-9) {
    double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (nearlyEq(cross, 0.0, eps))
      return 0;
    return (cross > 0.0) ? 1 : -1;
  }

  static bool onSegment(const Point& seg_start,
                        const Point& query,
                        const Point& seg_end,
                        double eps = 1e-9) {
    return query.x >= std::min(seg_start.x, seg_end.x) - eps &&
           query.x <= std::max(seg_start.x, seg_end.x) + eps &&
           query.y >= std::min(seg_start.y, seg_end.y) - eps &&
           query.y <= std::max(seg_start.y, seg_end.y) + eps;
  }

  static bool segmentsIntersect(const Point& a1,
                                const Point& a2,
                                const Point& b1,
                                const Point& b2,
                                double eps = 1e-9) {
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

 public:
  auto begin() { return points.begin(); }
  auto end() { return points.end(); }
  auto begin() const { return points.begin(); }
  auto end() const { return points.end(); }

  Point& operator[](std::size_t i) { return points[i]; }
  const Point& operator[](std::size_t i) const { return points[i]; }

  Point& at(std::size_t i) {
    if (i >= points.size())
      throw std::out_of_range("WorldPath::at");
    return points[i];
  }

  const Point& at(std::size_t i) const {
    if (i >= points.size())
      throw std::out_of_range("WorldPath::at");
    return points[i];
  }

  Point& front() {
    assert(!points.empty());
    return points.front();
  }
  const Point& front() const {
    assert(!points.empty());
    return points.front();
  }
  Point& back() {
    assert(!points.empty());
    return points.back();
  }
  const Point& back() const {
    assert(!points.empty());
    return points.back();
  }

  [[nodiscard]] bool empty() const { return points.empty(); }
  std::size_t size() const { return points.size(); }
  void reserve(std::size_t n) { points.reserve(n); }
  void clear() { points.clear(); }

  void addPoint(const Point& p) {
    assert(isValidPoint(p));
    points.push_back(p);
  }

  [[nodiscard]] bool popBack() {
    if (points.empty())
      return false;
    points.pop_back();
    return true;
  }

  std::span<const Point> pointsView() const { return points; }
  auto segments() const { return points | std::views::pairwise; }

  bool isValid() const { return std::ranges::all_of(points, isValidPoint); }

  double totalLength() const {
    return std::ranges::fold_left(
        segments(), 0.0, [](double acc, const auto& seg) {
          return acc + dist(std::get<0>(seg), std::get<1>(seg));
        });
  }

  std::optional<double> segmentLength(std::size_t i) const {
    if (i + 1 >= points.size())
      return std::nullopt;
    return dist(points[i], points[i + 1]);
  }

  std::pair<Point, Point> furthestPair() const {
    assert(points.size() >= 2);
    std::size_t ai = 0;
    std::size_t bi = 1;
    double maxDist = dist(points[ai], points[bi]);
    for (std::size_t i = 0; i < points.size(); ++i) {
      for (std::size_t j = i + 1; j < points.size(); ++j) {
        double d = dist(points[i], points[j]);
        if (d > maxDist) {
          maxDist = d;
          ai = i;
          bi = j;
        }
      }
    }
    return {points[ai], points[bi]};
  }

  bool isClosed(double eps = 1e-9) const {
    return points.size() >= 2 && samePoint(points.front(), points.back(), eps);
  }

  void append(const WorldPath& other) {
    points.insert(points.end(), other.points.begin(), other.points.end());
  }

  WorldPath reversed() const {
    WorldPath rev;
    rev.points.assign(points.rbegin(), points.rend());
    return rev;
  }

  Point pointAtDistance(double target) const {
    assert(!points.empty());
    if (target <= 0.0) {
      return points.front();
    }

    double traveled = 0.0;
    for (auto [a, b] : segments()) {
      double len = dist(a, b);
      if (len > 0.0 && traveled + len >= target) {
        double t = (target - traveled) / len;
        return {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
      }
      traveled += len;
    }
    return points.back();
  }

  bool selfIntersects() const {
    const std::size_t n = points.size();
    if (n < 4)
      return false;
    constexpr double eps = 1e-9;
    bool closed = samePoint(points.front(), points.back(), eps);
    for (std::size_t i = 0; i + 1 < n; ++i) {
      for (std::size_t j = i + 2; j + 1 < n; ++j) {
        if (closed && i == 0 && j == n - 2)
          continue;
        if (segmentsIntersect(points[i], points[i + 1], points[j],
                              points[j + 1], eps))
          return true;
      }
    }
    return false;
  }
};
