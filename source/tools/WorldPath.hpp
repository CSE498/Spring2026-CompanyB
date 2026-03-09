/// @file WorldPath.hpp
/// @author Devansh Tayal
/// @brief A list of ordered points representing an agent's path.

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
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

  WorldPath() = default;

  auto begin() { return points_.begin(); }
  auto end() { return points_.end(); }
  auto begin() const { return points_.begin(); }
  auto end() const { return points_.end(); }

  Point& operator[](std::size_t i) { return points_[i]; }
  const Point& operator[](std::size_t i) const { return points_[i]; }

  /**
   * @brief Bounds-checked access to the i-th point.
   * @param i Index of the point.
   * @return Reference to the point at index i.
   * @throws std::out_of_range if i >= size().
   */
  [[deprecated("Exceptions are disallowed. Use get() for checked access or [] for unchecked access.")]] Point& at(std::size_t i) {
    if (i >= points_.size())
      throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  /** @copydoc at(std::size_t) */
  [[deprecated("Exceptions are disallowed. Use get() for checked access or [] for unchecked access.")]] const Point& at(std::size_t i) const {
    if (i >= points_.size())
      throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  [[nodiscard]] Point* get(std::size_t i) noexcept {
    return i < points_.size() ? &points_[i] : nullptr;
  }

  [[nodiscard]] const Point* get(std::size_t i) const noexcept {
    return i < points_.size() ? &points_[i] : nullptr;
  }

  Point& front() {
    assert(!points_.empty());
    return points_.front();
  }
  const Point& front() const {
    assert(!points_.empty());
    return points_.front();
  }
  Point& back() {
    assert(!points_.empty());
    return points_.back();
  }
  const Point& back() const {
    assert(!points_.empty());
    return points_.back();
  }

  [[nodiscard]] bool empty() const { return points_.empty(); }
  [[nodiscard]] std::size_t size() const { return points_.size(); }
  void reserve(std::size_t n) { points_.reserve(n); }
  void clear() { points_.clear(); }

  /**
   * @brief Appends a point to the path.
   * @param p Point to add. Must have finite coordinates (asserts on NaN/Inf).
   */
  void addPoint(const Point& p);

  /**
   * @brief Removes the last point and returns it.
   * @return The removed point, or std::nullopt if the path was already empty.
   */
  std::optional<Point> popBack() {
    if (points_.empty())
      return std::nullopt;
    Point p = points_.back();
    points_.pop_back();
    return p;
  }

  /**
   * @brief Read-only span over the internal point storage.
   * @return A std::span of const Points.
   */
  [[nodiscard]] std::span<const Point> pointsView() const { return points_; }

  /**
   * @brief Consecutive point pairs along the path.
   * @return A pairwise view of the points.
   *
   * Example usage with structured bindings:
   * @code
   *   for (auto&& [start, end] : path.segments()) {
   *     double len = dist(start, end);
   *   }
   * @endcode
   */
  [[nodiscard]] auto segments() const {
    return points_ | std::views::pairwise;
  }

  /**
   * @brief Checks that every point has finite coordinates.
   * @return true if all points are finite, false otherwise.
   */
  [[nodiscard]] bool isValid() const {
    return std::ranges::all_of(points_, isValidPoint);
  }

  /**
   * @brief Total path length (sum of segment distances).
   * @return 0.0 for empty or single-point paths.
   */
  [[nodiscard]] double totalLength() const;

  /**
   * @brief Length of segment i (distance from point i to point i+1).
   * @param i Segment index.
   * @return The segment length, or std::nullopt if i+1 is out of range.
   */
  [[nodiscard]] std::optional<double> segmentLength(std::size_t i) const;

  /**
   * @brief Brute-force O(n^2) search for the two most distant points.
   * @return The two points with the greatest Euclidean distance.
   *
   * @note For large paths (millions of points), we will implement a more efficient
   *       algorithm such as rotating calipers on the convex hull (O(n log n)) ASAP.
   */
  [[nodiscard]] std::pair<Point, Point> furthestPair() const;

  /**
   * @brief Checks if the path forms a closed loop.
   * @param eps Tolerance for comparing the first and last points.
   * @return true if start and end points are the same within eps.
   */
  [[nodiscard]] bool isClosed(double eps = kDefaultEps) const;

  /**
   * @brief Tacks another path's points onto the end of this one.
   * @param other Path whose points will be appended.
   */
  void append(const WorldPath& other);

  /**
   * @brief Returns a copy with points in reverse order.
   * @return A new WorldPath with reversed point order.
   */
  [[nodiscard]] WorldPath reversed() const;

  /**
   * @brief Get the interpolated point at a given distance along the path.
   * @param distance_along_path Distance along the path to sample at.
   * @return The interpolated point. Clamps to first/last point if out of range.
   */
  [[nodiscard]] Point pointAtDistance(double distance_along_path) const;

  /**
   * @brief O(n^2) check for any two non-adjacent segments crossing.
   *
   * Could be improved with a sweep-line algorithm for large paths.
   *
   * @return true if any non-adjacent segments intersect.
   */
  [[nodiscard]] bool selfIntersects() const;

 private:
  std::vector<Point> points_{};

  // Geometry Helpers
  // These are declared here so the class encapsulates its own logic,
  // but implemented in the .cpp to keep the header clean.

  /// @brief Validates that a point's coordinates are not NaN or Infinity.
  [[nodiscard]] static bool isValidPoint(const Point& p);

  /// @brief Returns the Euclidean distance between points a and b.
  [[nodiscard]] static double dist(const Point& a, const Point& b);

  /// @brief Checks if two doubles are equal within a small tolerance.
  [[nodiscard]] static bool nearlyEq(double a, double b,
                                     double eps = kDefaultEps);

  /// @brief Checks if two points share the exact same coordinates (within eps).
  [[nodiscard]] static bool samePoint(const Point& a,
                                      const Point& b,
                                      double eps = kDefaultEps);

  /// @brief Finds the orientation of the ordered triplet (a, b, c).
  /// @return 0 if collinear, 1 if clockwise, -1 if counterclockwise.
  [[nodiscard]] static int orient(const Point& a,
                                  const Point& b,
                                  const Point& c,
                                  double eps = kDefaultEps);

  /// @brief Checks if a collinear point query lies on the line segment
  /// seg_start-seg_end.
  [[nodiscard]] static bool onSegment(const Point& seg_start,
                                      const Point& query,
                                      const Point& seg_end,
                                      double eps = kDefaultEps);

  /// @brief Returns true if line segment a1-a2 intersects with b1-b2.
  [[nodiscard]] static bool segmentsIntersect(const Point& a1,
                                              const Point& a2,
                                              const Point& b1,
                                              const Point& b2,
                                              double eps = kDefaultEps);
};

}  // namespace cse498
