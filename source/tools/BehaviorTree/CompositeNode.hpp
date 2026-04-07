#pragma once

#include <algorithm>

#include "Node.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Abstract base class for composite nodes in a Behavior Tree.
 *
 * A CompositeNode manages multiple child nodes and defines how
 * they are executed in relation to each other. Common examples
 * include SequenceNode and SelectNode (Selector), which
 * implement different execution policies over their children.
 *
 * Composite nodes maintain a list of children and propagate ticks
 * according to their specific logic. Derived classes must implement
 * tick() and getActivePath() to define their execution behavior.
 */
class CompositeNode : public Node {
 public:
  using Node::Node;
  virtual ~CompositeNode() = default;

  /**
   * @brief Adds a child node to this composite.
   *
   * Ownership of the node is transferred via a unique_ptr.
   *
   * @param node The node to add as a child.
   */
  void addNode(std::unique_ptr<Node> node) override {
    // assert(node.get() != nullptr); // Check for Nullptr

    m_children.push_back(std::move(node));
  };

  /**
   * @brief Removes a specific child node from this composite.
   *
   * The child is destroyed when removed.
   *
   * @param node Pointer to the child node to remove.
   */

  // ATTRIBUTIONS: Used ChatGPT to get the remove_if algorithm implementation

  void deleteNode(Node* node) override {
    std::erase_if(m_children, [node](const std::unique_ptr<Node>& child) {
      return child.get() == node;
    });
  }

  // ATTRIBUTIONS: Used ChatGPT to get ASCII implementation

  virtual void print(const std::string& prefix, bool isLast, bool isRoot) const {
    if (!isRoot) {
        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
    }
    std::cout << m_name << " (" << m_status << ")" << '\n';

    std::string newPrefix = prefix + (isLast ? "    " : "│   ");

    for (size_t i = 0; i < m_children.size(); ++i) {
        m_children[i]->print(newPrefix, i == m_children.size() - 1);
    }
  }

  virtual Status tick(Blackboard& blackboard) = 0;
  virtual std::string getActivePath() const = 0;

 protected:
  /**
   * @brief Provides read-only access to the child nodes.
   *
   * @return A const reference to the vector of child nodes.
   */
  const std::vector<std::unique_ptr<Node>>& getChildren() const {
    return m_children;
  }

 private:
  /// Owning pointers to all child nodes of this composite.
  std::vector<std::unique_ptr<Node>> m_children;
};