#pragma once

#include "Node.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Abstract base class representing terminal nodes in a Behavior Tree.
 *
 * A LeafNode is a node with no children and serves as the execution
 * endpoint within a behavior tree. Unlike composite nodes (which
 * manage multiple children) or decorator nodes (which modify the
 * behavior of a single child), leaf nodes implement concrete logic
 * directly.
 */
class LeafNode : public Node {
 public:
  using Node::Node;

  /**
   * @brief Constructs a leaf node with a name and tick duration.
   *
   * @param name Human-readable name of the node.
   * @param tickDuration Number of ticks required before the
   *        node transitions from Running to a terminal state.
   */
  LeafNode(std::string name, int tickDuration)
      : Node(name), m_tickDuration(tickDuration) {}
  virtual ~LeafNode() = default;

  virtual void print(int depth) const {
    std::cout << std::string(depth * 2, ' ') << m_name << " (" << m_status
              << "): " << m_tickDuration << '\n';
  };

  virtual Status tick() = 0;
  virtual std::string getActivePath() = 0;

 protected:
  int m_tickDuration{};

 private:
};