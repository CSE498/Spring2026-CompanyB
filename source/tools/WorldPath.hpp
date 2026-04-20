/// @file WorldPath.hpp
/// @author Devansh Tayal
/// @brief A list of ordered points representing an agent's path.

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/zip.hpp>
#include <span>
#include <stdexcept>
#include <vector>

#include "Point.hpp"

namespace cse498 {

/**
 * @brief Ordered list of 2-D waypoints that an agent can follow.
 *
 * Includes helpers for measuring length, checking self-intersection,
 * interpolating along the path, etc.
 *
 * @note Point is a small value type (two doubles, 16 bytes), so many helper
 * APIs pass it by value. For cheap-to-copy input types, pass-by-value is often
 * as good as or better than const reference because it can avoid an extra
 * indirection, depending on the optimizer and ABI. However, public-facing
 * methods (like addPoint) strictly retain the const Point& convention to
 * guarantee seamless API compatibility with the broader project ecosystem.
 */
class WorldPath {
 public:
  /// Tolerance for floating-point comparisons in geometry helpers.
  static constexpr double kDefaultEps = 1e-9;

  /// @brief Construct an empty path.
  constexpr WorldPath() noexcept = default;

  /// @brief Mutable iterator to the first point.
  constexpr auto begin() noexcept { return points_.begin(); }

  /// @brief Mutable iterator one past the last point.
  constexpr auto end() noexcept { return points_.end(); }

  /// @brief Const iterator to the first point.
  constexpr auto begin() const noexcept { return points_.begin(); }

  /// @brief Const iterator one past the last point.
  constexpr auto end() const noexcept { return points_.end(); }

  /**
   * @brief Unchecked mutable indexed access.
   * @param i Point index.
   * @return Mutable reference to the requested point.
   */
  constexpr Point& operator[](std::size_t i) noexcept { return points_[i]; }

  /**
   * @brief Unchecked const indexed access.
   * @param i Point index.
   * @return Const reference to the requested point.
   */
  constexpr const Point& operator[](std::size_t i) const noexcept {
    return points_[i];
  }

  /**
   * @brief Bounds-checked access to the i-th point.
   * @param i Index of the point.
   * @return Reference to the point at index i.
   * @throws std::out_of_range if i >= size().
   */
  [[deprecated(
      "Exceptions are disallowed. Use get() for checked access or [] for "
      "unchecked access.")]] Point&
  at(std::size_t i) {
    if (i >= points_.size()) throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  /** @copydoc at(std::size_t) */
  [[deprecated(
      "Exceptions are disallowed. Use get() for checked access or [] for "
      "unchecked access.")]] const Point&
  at(std::size_t i) const {
    if (i >= points_.size()) throw std::out_of_range("WorldPath::at");
    return points_[i];
  }

  /**
   * @brief Checked mutable indexed access.
   * @param i Point index.
   * @return Pointer to the point, or nullptr when i is out of range.
   */
  [[nodiscard]] constexpr Point* get(std::size_t i) noexcept {
    return i < points_.size() ? &points_[i] : nullptr;
  }

  /**
   * @brief Checked const indexed access.
   * @param i Point index.
   * @return Pointer to the point, or nullptr when i is out of range.
   */
  [[nodiscard]] constexpr const Point* get(std::size_t i) const noexcept {
    return i < points_.size() ? &points_[i] : nullptr;
  }

  /**
   * @brief Access the first point.
   * @return Mutable reference to the first point.
   * @pre Path must not be empty.
   */
  [[nodiscard]] constexpr Point& front() noexcept {
    assert(!points_.empty());
    return points_.front();
  }

  /**
   * @brief Access the first point.
   * @return Const reference to the first point.
   * @pre Path must not be empty.
   */
  [[nodiscard]] constexpr const Point& front() const noexcept {
    assert(!points_.empty());
    return points_.front();
  }

  /**
   * @brief Access the last point.
   * @return Mutable reference to the last point.
   * @pre Path must not be empty.
   */
  [[nodiscard]] constexpr Point& back() noexcept {
    assert(!points_.empty());
    return points_.back();
  }

  /**
   * @brief Access the last point.
   * @return Const reference to the last point.
   * @pre Path must not be empty.
   */
  [[nodiscard]] constexpr const Point& back() const noexcept {
    assert(!points_.empty());
    return points_.back();
  }

  /**
   * @brief Check whether the path has no points.
   * @return true if size() == 0.
   */
  [[nodiscard]] constexpr bool empty() const noexcept {
    return points_.empty();
  }

  /**
   * @brief Get the number of stored points.
   * @return Point count.
   */
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return points_.size();
  }

  /**
   * @brief Reserve storage for at least n points.
   * @param n Requested capacity.
   */
  constexpr void reserve(std::size_t n) { points_.reserve(n); }

  /**
   * @brief Remove all points from the path.
   */
  constexpr void clear() noexcept { points_.clear(); }

  /**
   * @brief Appends a point to the path.
   * @param p Point to add. Must have finite coordinates (asserts on NaN/Inf).
   */
  void addPoint(const Point& p);

  /**
   * @brief Removes the last point and returns it.
   * @return The removed point, or std::nullopt if the path was already empty.
   */
  constexpr std::optional<Point> popBack() noexcept {
    if (points_.empty()) return std::nullopt;
    Point p = points_.back();
    points_.pop_back();
    return p;
  }

  /**
   * @brief Read-only span over the internal point storage.
   * @return A std::span of const Points.
   */
  [[nodiscard]] constexpr std::span<const Point> pointsView() const noexcept {
    return points_;
  }

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
  [[nodiscard]] constexpr auto segments() const {
    return ranges::views::zip(points_, points_ | ranges::views::drop(1));
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
  [[nodiscard]] std::optional<double> segmentLengthAt(std::size_t i) const;

  /**
   * @brief Length of the subpath between two point indices.
   * @param start_idx Index of the start point.
   * @param end_idx Index of the end point.
   * @return The distance along the path, or std::nullopt if indices are invalid
   * or reversed.
   */
  [[nodiscard]] std::optional<double> subpathLength(std::size_t start_idx,
                                                    std::size_t end_idx) const;

  /**
   * @brief Finds the two points furthest away from each other.
   *        Uses a Convex Hull + Rotating Calipers algorithm to run fast (O(n
   * log n)).
   * @return A pair of the two most distant points.
   */
  [[nodiscard]] std::pair<Point, Point> furthestPair() const;

  /**
   * @brief O(n) check for adjacent segment foldbacks (path doubling back on
   * itself).
   * @return true if any sequence of 3 points strictly overlaps backward into
   * itself.
   */
  [[nodiscard]] bool hasFoldbacks() const;

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
  [[nodiscard]] static bool isValidPoint(Point p);

  /// @brief Returns the Euclidean distance between points a and b.
  [[nodiscard]] static double dist(Point a, Point b);

  /// @brief Checks if two doubles are equal within a small tolerance.
  [[nodiscard]] static bool nearlyEq(double a, double b,
                                     double eps = kDefaultEps);

  /// @brief Checks if two points share the exact same coordinates (within eps).
  [[nodiscard]] static bool samePoint(Point a, Point b,
                                      double eps = kDefaultEps);

  /// @brief 2D cross product of vectors AB and AC.
  [[nodiscard]] static double crossProduct(Point a, Point b, Point c);

  /// @brief Finds the orientation of the ordered triplet (a, b, c).
  /// @return 0 if collinear, 1 if clockwise, -1 if counterclockwise.
  [[nodiscard]] static int orient(Point a, Point b, Point c,
                                  double eps = kDefaultEps);

  /// @brief Checks if a collinear point query lies on the line segment
  /// seg_start-seg_end.
  [[nodiscard]] static bool onSegment(Point seg_start, Point query,
                                      Point seg_end, double eps = kDefaultEps);

  /// @brief Returns true if line segment a1-a2 intersects with b1-b2.
  [[nodiscard]] static bool segmentsIntersect(Point a1, Point a2, Point b1,
                                              Point b2,
                                              double eps = kDefaultEps);
};

}  // namespace cse498
