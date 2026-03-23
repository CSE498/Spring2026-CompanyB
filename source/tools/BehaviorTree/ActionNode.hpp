#pragma once

#include "LeafNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Leaf node that performs a concrete action.
 *
 * An ActionNode represents an executable behavior at the leaf
 * level of a behavior tree. Unlike composite or decorator nodes,
 * it does not have children and encapsulates a specific action
 * or task.
 */
class ActionNode : public LeafNode {
 public:
  using LeafNode::LeafNode;

  Status tick() override {
    ++m_tickCount;

    //std::cout << m_name << '\n';

    m_status = (m_tickDuration > 1) ? Status::Running : Status::Success;

    if ((m_tickDuration > 0)) --m_tickDuration;

    return m_status;
  };

  int tickCount() const { return m_tickCount; }

  std::string getActivePath() override { return m_name; }

 private:
  int m_tickCount{};
};