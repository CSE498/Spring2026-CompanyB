#pragma once

#include <expected>
#include <iostream>
#include <memory>

#include "ActionNode.hpp"
#include "Blackboard.hpp"
#include "InvertNode.hpp"
#include "RepeatNode.hpp"
#include "SelectNode.hpp"
#include "SequenceNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

namespace cse498 {

/**
 * @enum NodeType
 * @brief Enumerates the types of parent (non-leaf) behavior tree nodes.
 *
 * These values are used by NodeFactory to determine which concrete
 * composite or decorator node to construct.
 */
enum class NodeType { Sequence, Select, Invert, Repeat };

/**
 * @brief Represents the root container and execution driver for a Behavior
 * Tree.
 *
 * The BehaviorTree owns the root Node and is responsible for
 * advancing execution through calls to tick(). It also provides
 * utilities for debugging and introspection, such as retrieving
 * the currently active execution path and visualizing the tree.
 */
class BehaviorTree {
 public:
  /**
   * @brief Constructs a BehaviorTree with the given root node.
   *
   * Transfers ownership of the provided root node into the tree.
   *
   * The root must not be nullptr.
   *
   * @param root Unique pointer to the root Node of the tree.
   */
  BehaviorTree(std::unique_ptr<Node> root) : m_root(std::move(root)) {
    assert(m_root != nullptr);
  }

  /**
   * @brief Advances the behavior tree by one tick.
   *
   * Calls tick() on the root node, propagating execution
   * through the tree according to node semantics.
   *
   * Increments the internal tick counter.
   *
   * @return The status code returned by the root node
   *         (e.g., Success (1), Failure (0), or Running (-1)).
   */
  Status tick(Blackboard& blackboard) {
    ++m_tickCount;
    return m_root->tick(blackboard);
  }

  /**
   * @brief Returns the total number of times the tree has been ticked.
   *
   * @return The accumulated tick count.
   */
  int tickCount() const { return m_tickCount; }

  /**
   * @brief Retrieves the currently active execution path.
   *
   * Traverses the tree to determine which nodes are
   * currently active or running and returns a string
   * representation of that path.
   *
   * @return A formatted string representing the active node path.
   */
  std::string getActivePath() const { return m_root->getActivePath(); }

  Blackboard& getBlackboard() { return m_blackboard; }

  /**
   * @brief Outputs a debug visualization of the behavior tree.
   *
   * Traverses the tree structure and prints a formatted
   * representation of its nodes and current states.
   *
   * Intended for debugging and development purposes.
   */
  void debugView() const { m_root->print("", true, true); }

 private:
  /// Tracks the total number of tick() calls made on this tree.
  int m_tickCount{};

  /// Owning pointer to the root node of the behavior tree.
  std::unique_ptr<Node> m_root;

  Blackboard m_blackboard;
};

/**
 * @class NodeFactory
 * @brief Factory class for constructing behavior tree nodes.
 *
 * Provides static helper methods to create parent (composite/decorator)
 * and action (leaf) nodes. Encapsulates node creation logic and ensures
 * consistent use of smart pointers.
 */
class NodeFactory {
 public:
  /**
   * @brief Creates a parent node based on the given NodeType.
   *
   * This includes composite nodes (e.g., Sequence, Select) and decorator
   * nodes (e.g., Invert, Repeat).
   *
   * @param type The type of node to create.
   * @param name The name assigned to the node.
   * @return std::expected<std::unique_ptr<Node>, std::string>
   *         - On success: a unique_ptr to the created node.
   *         - On failure: an error message describing the issue.
   *
   * @note The caller assumes ownership of the returned node.
   * @note Returns std::unexpected if the NodeType is invalid.
   */
  static std::expected<std::unique_ptr<Node>, std::string> createParentNode(
      const NodeType& type, const std::string& name) {
    switch (type) {
      case NodeType::Sequence:
        return std::make_unique<SequenceNode>(name);
        break;

      case NodeType::Select:
        return std::make_unique<SelectNode>(name);
        break;

      case NodeType::Invert:
        return std::make_unique<InvertNode>(name);
        break;

      case NodeType::Repeat:
        return std::make_unique<RepeatNode>(name);
        break;

      default:
        return std::unexpected("Invalid Node Type");
    }
  }

  /**
   * @brief Creates an action (leaf) node.
   *
   * Action nodes encapsulate executable behavior and do not have children.
   *
   * @param name The name assigned to the node.
   * @param action The callable representing the node's behavior.
   * @param tickDuration The number of ticks the action should run.
   * @return std::unique_ptr<Node> A unique_ptr to the created ActionNode.
   *
   * @note The caller assumes ownership of the returned node.
   */
  static std::unique_ptr<Node> createActionNode(const std::string& name,
                                                const Action& action,
                                                const int& tickDuration) {
    return std::make_unique<ActionNode>(name, action, tickDuration);
  }
};

}  // namespace cse498