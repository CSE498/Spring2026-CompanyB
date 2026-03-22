#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/// Possible statuses of the nodes
enum class Status { Success = 1, Failure = 0, Running = -1 };

// Overload stream insertion operator for the Status enum class
inline std::ostream& operator<<(std::ostream& os, const Status& status) {
  switch (status) {
    case Status::Success:
      os << "1";
      break;
    case Status::Failure:
      os << "0";
      break;
    case Status::Running:
      os << "-1";
      break;
    default:
      os << "NaN";
      break;
  }
  return os;
}

/**
 * @brief Abstract base class representing a node in a Behavior Tree.
 *
 * Node defines the common interface for all behavior tree nodes,
 * including composites (e.g., Sequence, Selector), decorators,
 * and leaf/action nodes.
 *
 * Each node has a name used for identification and debugging.
 * Derived classes must implement execution (tick), active-path
 * reporting, and formatted printing behavior.
 */
class Node {
 public:
  /// Human-readable name of the node (used for identification, debugging, and
  /// visualization).
  std::string m_name;

  /**
   * @brief Constructs a node with the given name.
   *
   * @param name A descriptive name for the node.
   */
  Node(std::string name) : m_name(std::move(name)) {}

  virtual ~Node() = default;

  /**
   * @brief Prints a formatted representation of the node.
   *
   * Typically used for debugging or visualization. The depth
   * parameter indicates the node's level in the tree hierarchy
   * and is commonly used to control indentation.
   *
   * @param depth The current depth within the tree.
   */
  virtual void print(int depth) const = 0;

  /**
   * @brief Returns the active execution path from this node downward.
   *
   * Used to determine which nodes are currently active or running.
   * Implementations should return a formatted string representing
   * the path through active children.
   *
   * @return A string describing the active path.
   */
  virtual std::string getActivePath() = 0;

  /**
   * @brief Executes one tick of this node.
   *
   * This function defines the core behavior of the node.
   * Composite nodes propagate ticks to their children,
   * decorators modify child behavior, and leaf nodes
   * perform concrete actions.
   *
   * @return An integer status code (e.g., Success (1), Failure (0), or Running
   * (-1)).
   */
  virtual Status tick() = 0;

 protected:
  /// Stores the current execution status of the node.
  Status m_status = Status::Running;
};