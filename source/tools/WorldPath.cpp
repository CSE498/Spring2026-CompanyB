#include "WorldPath.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace cse498 {

bool WorldPath::isValidPoint(const Point& p) {
  return std::isfinite(p.x()) && std::isfinite(p.y());
}

double WorldPath::dist(const Point& a, const Point& b) {
  double dx = b.x() - a.x();
  double dy = b.y() - a.y();
  return std::sqrt(dx * dx + dy * dy);
}

bool WorldPath::nearlyEq(double a, double b, double eps) {
  return std::abs(a - b) <= eps;
}

bool WorldPath::samePoint(const Point& a, const Point& b, double eps) {
  return nearlyEq(a.x(), b.x(), eps) && nearlyEq(a.y(), b.y(), eps);
}

int WorldPath::orient(const Point& a,
                      const Point& b,
                      const Point& c,
                      double eps) {
  // Calculate 2D cross product to determine turn direction.
  // Positive = CCW, Negative = CW, Zero = collinear.
  double cross =
      (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
  if (nearlyEq(cross, 0.0, eps))
    return 0;
  return (cross > 0.0) ? 1 : -1;
}

bool WorldPath::onSegment(const Point& seg_start,
                          const Point& query,
                          const Point& seg_end,
                          double eps) {
  return query.x() >= std::min(seg_start.x(), seg_end.x()) - eps &&
         query.x() <= std::max(seg_start.x(), seg_end.x()) + eps &&
         query.y() >= std::min(seg_start.y(), seg_end.y()) - eps &&
         query.y() <= std::max(seg_start.y(), seg_end.y()) + eps;
}

bool WorldPath::segmentsIntersect(const Point& a1,
                                  const Point& a2,
                                  const Point& b1,
                                  const Point& b2,
                                  double eps) {
  int d1 = orient(a1, a2, b1, eps);
  int d2 = orient(a1, a2, b2, eps);
  int d3 = orient(b1, b2, a1, eps);
  int d4 = orient(b1, b2, a2, eps);
  if (d1 != d2 && d3 != d4)
    return true;  // Strictly intersect

  // Check for collinear overlaps
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

void WorldPath::addPoint(const Point& p) {
  assert(isValidPoint(p));
  points_.push_back(p);
}

double WorldPath::totalLength() const {
  // fold_left aggregates the distances of all consecutive point pairs
  // (segments)
  return std::ranges::fold_left(
      segments(), 0.0, [](double acc, const auto& seg) {
        return acc + dist(std::get<0>(seg), std::get<1>(seg));
      });
}

std::optional<double> WorldPath::segmentLength(std::size_t i) const {
  if (i + 1 >= points_.size())
    return std::nullopt;
  return dist(points_[i], points_[i + 1]);
}

std::pair<Point, Point> WorldPath::furthestPair() const {
  assert(points_.size() >= 2);
  std::size_t ai = 0;
  std::size_t bi = 1;
  double best = dist(points_[ai], points_[bi]);

  // Brute-force O(n^2) comparison of all pairs
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

bool WorldPath::isClosed(double eps) const {
  return points_.size() >= 2 && samePoint(points_.front(), points_.back(), eps);
}

void WorldPath::append(const WorldPath& other) {
  points_.insert(points_.end(), other.points_.begin(), other.points_.end());
}

WorldPath WorldPath::reversed() const {
  WorldPath rev;
  rev.points_.assign(points_.rbegin(), points_.rend());
  return rev;
}

Point WorldPath::pointAtDistance(double distance_along_path) const {
  assert(!points_.empty());
  if (distance_along_path <= 0.0)
    return points_.front();

  double remaining = distance_along_path;

  for (auto&& [start, end] : segments()) {
    const double segment_length = dist(start, end);
    if (segment_length <= 0.0)
      continue;  // skip duplicate points

    if (remaining <= segment_length) {
      const double t = remaining / segment_length;
      return {start.x() + t * (end.x() - start.x()),
              start.y() + t * (end.y() - start.y())};
    }

    remaining -= segment_length;
  }

  return points_.back();  // clamp past-the-end distances
}

bool WorldPath::selfIntersects() const {
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

}  // namespace cse498