/// @file PathGenerator.hpp
/// @author Group 11 (Chirag Bhansali, Lalit Chitibomma, Daniel Petkoski, Cole
/// Scheller, Devansh Tayal)
/// @brief Pathfinding and path generation utilities for agent navigation.

// NOTE - Some methods and comments were written with the help of AI

#pragma once

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Math/Point.hpp"
#include "WorldPath.hpp"

namespace cse498 {

using Point = Math::Point;

/// Type alias for world query callback functions
using WorldQueryFunc = std::function<bool(const Point &)>;

/// Type alias for distance heuristic callback functions
using HeuristicFunc = std::function<double(const Point &, const Point &)>;

/**
 * @brief Pathfinding and path generation for agent navigation.
 *
 * Provides A* pathfinding, patrol routes, avoidance paths, random walks,
 * and procedural path generation (spirals). Configurable with custom
 * heuristics and step sizes for different world representations.
 */
class PathGenerator {
public:
  // Fraction of step_size_ below which two points are treated as coincident
  static constexpr double kPointCoincidentFraction = 0.01;
  // Fraction of step_size_ within which a node is considered to have reached
  // the goal
  static constexpr double kGoalReachedFraction = 0.6;

  /**
   * @brief Construct a PathGenerator with default settings.
   *
   * Default heuristic: Euclidean distance
   * Default step size: 1.0
   */
  PathGenerator();

  // ========== Core Path Generation ==========

  /**
   * @brief Generate shortest path from start to goal using A* algorithm.
   * @param start Starting point
   * @param goal Goal point
   * @param canMove Callback to check if a point is traversable
   * @return Optional WorldPath (nullopt if no path exists)
   */
  [[nodiscard]] std::optional<WorldPath>
  ShortestPath(const Point &start, const Point &goal,
               const WorldQueryFunc &canMove) const;

  /**
   * @brief Generate a patrol path through waypoints.
   * @param waypoints Points to visit in order
   * @param loop If true, return to start point
   * @return Optional WorldPath (nullopt if waypoints are unreachable)
   */
  [[nodiscard]] std::optional<WorldPath>
  PatrolPath(const std::vector<Point> &waypoints, bool loop = true) const;

  /**
   * @brief Generate path from start to goal while avoiding a region.
   * @param start Starting point
   * @param goal Goal point
   * @param avoid Center of region to avoid
   * @param radius Radius around avoid point to stay away from
   * @param canMove Callback to check if a point is traversable
   * @return Optional WorldPath (nullopt if avoidance makes path impossible)
   */
  [[nodiscard]] std::optional<WorldPath>
  AvoidancePath(const Point &start, const Point &goal, const Point &avoid,
                double radius, const WorldQueryFunc &canMove) const;

  // ========== Utility Generation ==========

  /**
   * @brief Generate a random walk path for exploration.
   * @param start Starting point
   * @param steps Number of steps to take
   * @param canMove Callback to check if a point is traversable
   * @return WorldPath (returns partial path if dead-end reached)
   */
  [[nodiscard]] WorldPath RandomWalk(const Point &start, size_t steps,
                                     const WorldQueryFunc &canMove) const;

  /**
   * @brief Generate an expanding spiral path centered at a point.
   * @param center Center of the spiral
   * @param spacing Distance between spiral turns
   * @param turns Number of complete rotations
   * @return WorldPath representing the spiral
   */
  [[nodiscard]] WorldPath SpiralPath(const Point &center, double spacing,
                                     size_t turns) const;

  // ========== Configuration ==========

  /**
   * @brief Set the distance heuristic for pathfinding algorithms.
   * @param h Heuristic function (e.g., Manhattan, Euclidean, Chebyshev)
   */
  void SetHeuristic(HeuristicFunc h);

  /**
   * @brief Set step size for continuous world pathfinding.
   * @param size Step size (must be positive)
   */
  void SetStepSize(double size);

  /**
   * @brief Get current step size.
   * @return Current step size value
   */
  [[nodiscard]] double GetStepSize() const { return step_size_; }

private:
  HeuristicFunc heuristic_; ///< Distance estimation function
  double step_size_;        ///< Granularity for continuous worlds

  // ========== Helper Functions ==========

  /**
   * @brief Default Euclidean distance heuristic.
   */
  static double EuclideanDistance(const Point &a, const Point &b);

  /**
   * @brief Get neighboring points for pathfinding expansion.
   * @param p Current point
   * @return Vector of neighboring points based on step_size_
   */
  [[nodiscard]] std::vector<Point> GetNeighbors(const Point &p) const;

  /**
   * @brief Reconstruct path from A* search results.
   * @param came_from Map of point -> previous point
   * @param current Final point reached
   * @return Reconstructed WorldPath
   */
  [[nodiscard]] WorldPath ReconstructPath(
      const std::unordered_map<Point, Point, Math::PointHash> &came_from,
      const Point &current) const;
};

} // namespace cse498
