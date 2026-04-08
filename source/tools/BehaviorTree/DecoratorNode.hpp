#pragma once

#include "Node.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Abstract base class for decorator nodes in a Behavior Tree.
 *
 * A DecoratorNode wraps a single child node and modifies or controls
 * its behavior in some way. Examples include inverting results,
 * repeating execution, or adding conditional checks.
 *
 * Decorators maintain a single child node and define how that child's
 * tick result affects the overall status of the decorator.
 */
class DecoratorNode : public Node {
 public:
  using Node::Node;
  virtual ~DecoratorNode() = default;

  /**
   * @brief Attaches a child node to this decorator.
   *
   * Only one child is allowed. Ownership of the node is transferred
   * via a unique_ptr.
   *
   * @param node The node to attach as the child.
   */
  void addNode(std::unique_ptr<Node> node) override {
    // assert(m_child != nullptr); // Check for Existing Child

    m_child = std::move(node);
  }

  /**
   * @brief Removes the child node from this decorator.
   *
   * The child is destroyed when removed. After deletion, the decorator
   * has no child until a new node is added.
   */
  void deleteNode([[maybe_unused]] Node* node) override { m_child = nullptr; }

  // ATTRIBUTIONS: Used ChatGPT to get ASCII implementation

  virtual void print(const std::string& prefix, bool isLast,
                     bool isRoot) const {
    if (!isRoot) {
      std::cout << prefix;
      std::cout << (isLast ? "└── " : "├── ");
    }
    std::cout << m_name << " (" << m_status << ")" << '\n';

    if (m_child) {
      std::string newPrefix = prefix + (isLast ? "    " : "│   ");
      m_child->print(newPrefix, true);
    }
  }

  virtual Status tick(Blackboard& blackboard) = 0;
  virtual std::string getActivePath() const = 0;

 protected:
  /**
   * @brief Provides read-only access to the child node.
   *
   * @return A const reference to the unique_ptr of the child node.
   */
  const std::unique_ptr<Node>& getChild() const { return m_child; }

 private:
  /// Owning pointer to the single child node of this decorator.
  std::unique_ptr<Node> m_child;
};