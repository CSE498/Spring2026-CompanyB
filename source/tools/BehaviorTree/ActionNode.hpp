#pragma once

#include <functional>

#include "Node.hpp"

using Action = std::function<Status(Blackboard&)>;

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
class ActionNode : public Node {
 public:
  using Node::Node;

  ActionNode(std::string name, Action action, int tickDuration)
      : Node(name), m_action(action), m_tickDuration(tickDuration) {}

  virtual void print(int depth) const {
    std::cout << std::string(depth * 2, ' ') << m_name << " (" << m_status
              << "): " << m_tickDuration << '\n';
  };

  Status tick(Blackboard& blackboard) override {
    ++m_tickCount;

    if (m_action) {
      m_action(blackboard);
    }
    else {
      return Status::Failure;
    }

    m_status = (m_tickDuration > 1) ? Status::Running : Status::Success;

    if ((m_tickDuration > 0)) --m_tickDuration;

    return m_status;
  };

  int tickCount() const { return m_tickCount; }

  std::string getActivePath() const override { return m_name; }

 private:
  Action m_action;
  int m_tickDuration{};
  int m_tickCount{};
};