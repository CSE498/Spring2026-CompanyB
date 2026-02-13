#pragma once

#include "Math/Point.hpp"
#include "WorldPath.hpp"
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

using Point = Math::Point;

// Type aliases for callback functions
using WorldQueryFunc = std::function<bool(const Point &)>;
using HeuristicFunc = std::function<double(const Point &, const Point &)>;

class PathGenerator {
public:
  /**
   * @brief Construct a PathGenerator with default settings
   *
   * Default heuristic: Euclidean distance
   * Default step size: 1.0
   */
  PathGenerator();

  // ========== Core Path Generation ==========

  /**
   * @brief Generate shortest path from start to goal using A* algorithm
   * @param start Starting point
   * @param goal Goal point
   * @param canMove Callback to check if a point is traversable
   * @return Optional WorldPath (nullopt if no path exists)
   */
  std::optional<WorldPath> ShortestPath(const Point &start, const Point &goal,
                                        WorldQueryFunc canMove);

  /**
   * @brief Generate a patrol path through waypoints
   * @param waypoints Points to visit in order
   * @param loop If true, return to start point
   * @return Optional WorldPath (nullopt if waypoints are unreachable)
   */
  std::optional<WorldPath> PatrolPath(const std::vector<Point> &waypoints,
                                      bool loop = true);

  /**
   * @brief Generate path from start to goal while avoiding a region
   * @param start Starting point
   * @param goal Goal point
   * @param avoid Center of region to avoid
   * @param radius Radius around avoid point to stay away from
   * @param canMove Callback to check if a point is traversable
   * @return Optional WorldPath (nullopt if avoidance makes path impossible)
   */
  std::optional<WorldPath> AvoidancePath(const Point &start, const Point &goal,
                                         const Point &avoid, double radius,
                                         WorldQueryFunc canMove);

  // ========== Utility Generation ==========

  /**
   * @brief Generate a random walk path for exploration
   * @param start Starting point
   * @param steps Number of steps to take
   * @param canMove Callback to check if a point is traversable
   * @return WorldPath (returns partial path if dead-end reached)
   */
  WorldPath RandomWalk(const Point &start, size_t steps,
                       WorldQueryFunc canMove);

  /**
   * @brief Generate an expanding spiral path centered at a point
   * @param center Center of the spiral
   * @param spacing Distance between spiral turns
   * @param turns Number of complete rotations
   * @return WorldPath representing the spiral
   */
  WorldPath SpiralPath(const Point &center, double spacing, size_t turns);

  // ========== Configuration ==========

  /**
   * @brief Set the distance heuristic for pathfinding algorithms
   * @param h Heuristic function (e.g., Manhattan, Euclidean, Chebyshev)
   */
  void SetHeuristic(HeuristicFunc h);

  /**
   * @brief Set step size for continuous world pathfinding
   * @param size Step size (must be positive)
   */
  void SetStepSize(double size);

  /**
   * @brief Get current step size
   * @return Current step size value
   */
  double GetStepSize() const { return step_size_; }

private:
  HeuristicFunc heuristic_; ///< Distance estimation function
  double step_size_;        ///< Granularity for continuous worlds

  // ========== Helper Functions ==========

  /**
   * @brief Default Euclidean distance heuristic
   */
  static double EuclideanDistance(const Point &a, const Point &b);

  /**
   * @brief Get neighboring points for pathfinding expansion
   * @param p Current point
   * @return Vector of neighboring points based on step_size_
   */
  std::vector<Point> GetNeighbors(const Point &p) const;

  /**
   * @brief Reconstruct path from A* search results
   * @param came_from Map of point -> previous point
   * @param current Final point reached
   * @return Reconstructed WorldPath
   */
  WorldPath ReconstructPath(
      const std::unordered_map<Point, Point, Math::PointHash> &came_from,
      const Point &current) const;
};