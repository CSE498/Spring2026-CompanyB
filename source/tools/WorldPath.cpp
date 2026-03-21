#include "WorldPath.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ranges>
#include <utility>

namespace cse498 {

bool WorldPath::isValidPoint(Point p) {
  return std::isfinite(p.getX()) && std::isfinite(p.getY());
}

double WorldPath::dist(Point a, Point b) {
  double dx = b.getX() - a.getX();
  double dy = b.getY() - a.getY();
  return std::sqrt(dx * dx + dy * dy);
}

bool WorldPath::nearlyEq(double a, double b, double eps) {
  return std::abs(a - b) <= eps;
}

bool WorldPath::samePoint(Point a, Point b, double eps) {
  return nearlyEq(a.getX(), b.getX(), eps) && nearlyEq(a.getY(), b.getY(), eps);
}

double WorldPath::crossProduct(Point a, Point b, Point c) {
  return (b.getX() - a.getX()) * (c.getY() - a.getY()) -
         (b.getY() - a.getY()) * (c.getX() - a.getX());
}

int WorldPath::orient(Point a, Point b, Point c, double eps) {
  // Calculate 2D cross product to determine turn direction.
  // Positive = CCW, Negative = CW, Zero = collinear.
  double cross = crossProduct(a, b, c);
  if (nearlyEq(cross, 0.0, eps))
    return 0;
  return (cross > 0.0) ? 1 : -1;
}

bool WorldPath::onSegment(Point seg_start,
                          Point query,
                          Point seg_end,
                          double eps) {
  return query.getX() >= std::min(seg_start.getX(), seg_end.getX()) - eps &&
         query.getX() <= std::max(seg_start.getX(), seg_end.getX()) + eps &&
         query.getY() >= std::min(seg_start.getY(), seg_end.getY()) - eps &&
         query.getY() <= std::max(seg_start.getY(), seg_end.getY()) + eps;
}

bool WorldPath::segmentsIntersect(Point a1,
                                  Point a2,
                                  Point b1,
                                  Point b2,
                                  double eps) {
  int d1 = orient(a1, a2, b1, eps);
  int d2 = orient(a1, a2, b2, eps);

  // If points b1 and b2 are strictly on the same side of line a1-a2,
  // they cannot intersect.
  if (d1 == d2 && d1 != 0)
    return false;

  // Quick strict-intersection check before collinear work
  int d3 = orient(b1, b2, a1, eps);
  int d4 = orient(b1, b2, a2, eps);

  // If points a1 and a2 are strictly on the same side of line b1-b2,
  // they cannot intersect.
  if (d3 == d4 && d3 != 0)
    return false;

  if (d1 != d2 && d3 != d4)
    return true;  // Strictly intersect

  // Collinear overlap checks (only reached when orientations match)
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

std::optional<double> WorldPath::segmentLengthAt(std::size_t i) const {
  if (i + 1 >= points_.size())
    return std::nullopt;
  return dist(points_[i], points_[i + 1]);
}

std::optional<double> WorldPath::subpathLength(std::size_t start_idx,
                                               std::size_t end_idx) const {
  if (start_idx >= points_.size() || end_idx >= points_.size() ||
      start_idx > end_idx)
    return std::nullopt;

  double total = 0.0;
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    total += dist(points_[i], points_[i + 1]);
  }
  return total;
}

std::pair<Point, Point> WorldPath::furthestPair() const {
  assert(points_.size() >= 2);
  if (points_.size() == 2)
    return {points_[0], points_[1]};

  // sort points by X (then Y)
  std::vector<Point> sorted = points_;
  std::ranges::sort(sorted, [](const Point& a, const Point& b) {
    if (std::abs(a.getX() - b.getX()) > kDefaultEps)
      return a.getX() < b.getX();
    return a.getY() < b.getY();
  });

  // drop duplicates so hull logic works
  auto unique_end =
      std::ranges::unique(sorted, [](const Point& a, const Point& b) {
        return std::abs(a.getX() - b.getX()) < kDefaultEps &&
               std::abs(a.getY() - b.getY()) < kDefaultEps;
      });
  sorted.erase(unique_end.begin(), unique_end.end());

  if (sorted.size() <= 2) {
    if (sorted.size() == 1)
      return {sorted[0], sorted[0]};
    return {sorted[0], sorted[1]};
  }

  // convex hull via Andrew's Monotone Chain
  // https://cp-algorithms.com/geometry/convex-hull.html
  std::vector<Point> hull;
  hull.reserve(sorted.size() + 1);

  // lower hull
  for (Point p : sorted) {
    while (hull.size() >= 2 &&
           crossProduct(hull[hull.size() - 2], hull.back(), p) <= kDefaultEps) {
      hull.pop_back();
    }
    hull.push_back(p);
  }

  // upper hull
  std::size_t lower_size = hull.size();
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    Point p = *it;
    while (hull.size() > lower_size &&
           crossProduct(hull[hull.size() - 2], hull.back(), p) <= kDefaultEps) {
      hull.pop_back();
    }
    hull.push_back(p);
  }
  hull.pop_back();  // last point is just a dup of the start point

  std::size_t H = hull.size();
  if (H <= 2) {
    return {hull.front(), hull.back()};
  }

  // rotating calipers to find max distance
  // http://web.archive.org/web/20120313054937/http://cgm.cs.mcgill.ca/~orm/rotcal.html
  double max_dist_sq = 0.0;
  std::size_t best_i = 0, best_j = 1;

  auto distSq = [](Point a, Point b) {
    double dx = a.getX() - b.getX();
    double dy = a.getY() - b.getY();
    return dx * dx + dy * dy;
  };

  std::size_t j = 1;
  for (std::size_t i = 0; i < H; ++i) {
    std::size_t next_i = (i + 1) % H;

    // advance j while area increases (moving further from edge i->next_i)
    while (true) {
      std::size_t next_j = (j + 1) % H;
      double area_j = std::abs(crossProduct(hull[i], hull[next_i], hull[j]));
      double area_next_j =
          std::abs(crossProduct(hull[i], hull[next_i], hull[next_j]));

      if (area_next_j > area_j + kDefaultEps) {
        j = next_j;
      } else {
        break;
      }
    }

    // check distances to j
    double d1 = distSq(hull[i], hull[j]);
    if (d1 > max_dist_sq) {
      max_dist_sq = d1;
      best_i = i;
      best_j = j;
    }

    double d2 = distSq(hull[next_i], hull[j]);
    if (d2 > max_dist_sq) {
      max_dist_sq = d2;
      best_i = next_i;
      best_j = j;
    }
  }

  return {hull[best_i], hull[best_j]};
}

bool WorldPath::hasFoldbacks() const {
  if (points_.size() < 3)
    return false;

  // 3-point observing window with slide
  for (const auto& window : points_ | std::views::slide(3)) {
    const Point& A = window[0];
    const Point& B = window[1];
    const Point& C = window[2];

    // Check if points are collinear
    if (orient(A, B, C, kDefaultEps) == 0) {
      // If the dot product of vectors BA and BC is distinctly positive,
      // the path is folding directly back onto itself.
      if (dot(A - B, C - B) > kDefaultEps)
        return true;
    }
  }
  return false;
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
      return {start.getX() + t * (end.getX() - start.getX()),
              start.getY() + t * (end.getY() - start.getY())};
    }

    remaining -= segment_length;
  }

  return points_.back();  // clamp past-the-end distances
}

bool WorldPath::selfIntersects() const {
  const std::size_t n = points_.size();
  // Need at least 4 points (3 segments) for any non-adjacent segment pair to
  // exist.
  if (n < 4)
    return false;

  bool closed = samePoint(points_.front(), points_.back(), kDefaultEps);

  struct SegNode {
    double min_x, max_x;
    double min_y, max_y;
    std::size_t index;
  };

  std::vector<SegNode> nodes;
  nodes.reserve(n - 1);
  for (std::size_t i = 0; i + 1 < n; ++i) {
    const Point& p1 = points_[i];
    const Point& p2 = points_[i + 1];
    nodes.push_back(SegNode{
        std::min(p1.getX(), p2.getX()), std::max(p1.getX(), p2.getX()),
        std::min(p1.getY(), p2.getY()), std::max(p1.getY(), p2.getY()), i});
  }

  std::sort(nodes.begin(), nodes.end(), [](const SegNode& a, const SegNode& b) {
    return a.min_x < b.min_x;
  });

  for (std::size_t i = 0; i < nodes.size(); ++i) {
    const auto& nodeA = nodes[i];
    for (std::size_t j = i + 1; j < nodes.size(); ++j) {
      const auto& nodeB = nodes[j];

      // Since nodes are sorted by min_x, if nodeB's min_x is strictly beyond
      // nodeA's max_x (plus eps for floating point safety), we can stop
      // checking nodeB and all subsequent nodes against nodeA.
      if (nodeB.min_x > nodeA.max_x + kDefaultEps) {
        break;
      }

      // Bounding box overlap check on Y axis
      if (nodeB.min_y > nodeA.max_y + kDefaultEps ||
          nodeB.max_y < nodeA.min_y - kDefaultEps) {
        continue;
      }

      // Determine adjacency
      std::size_t idxA = nodeA.index;
      std::size_t idxB = nodeB.index;

      if (idxA > idxB)
        std::swap(idxA, idxB);

      // Adjacent segments naturally intersect at their shared vertex
      if (idxB == idxA + 1)
        continue;

      // Skip the first-last segment pair for closed paths (they share a vertex)
      if (closed && idxA == 0 && idxB == n - 2)
        continue;

      // Exact intersection check
      if (segmentsIntersect(points_[idxA], points_[idxA + 1], points_[idxB],
                            points_[idxB + 1], kDefaultEps)) {
        return true;
      }
    }
  }

  return false;
}

}  // namespace cse498