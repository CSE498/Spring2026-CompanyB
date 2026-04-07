#pragma once

#include "DecoratorNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Decorator node that inverts its child's result.
 *
 * An InvertNode (also known as an Inverter) wraps a single child node
 * and reverses its success/failure outcome:
 *
 * - If the child returns Status::Success -> returns Status::Failure.
 * - If the child returns Status::Failure -> returns Status::Success.
 * - If the child returns Status::Running -> returns Status::Running
 * (unchanged).
 *
 * This decorator is commonly used to negate conditions
 * or reverse logical behavior in a behavior tree.
 */
class InvertNode : public DecoratorNode {
 public:
  using DecoratorNode::DecoratorNode;

  Status tick(Blackboard& blackboard) override {
    ++m_tickCount;

    // Check status of active child
    auto& child = this->getChild();
    if (!child) return Status::Running;

    Status status = child->tick(blackboard);

    if (status == Status::Running) {
      m_status = Status::Running;
      return m_status;
    }

    m_status = (status == Status::Success) ? Status::Failure : Status::Success;

    return m_status;
  };

  int tickCount() const { return m_tickCount; }

  std::string getActivePath() const override {
    auto& child = this->getChild();

    return (child) ? m_name + " - " + child->getActivePath() : m_name;
  };

 private:
  /// Tracks the total number of tick() calls made on this node.
  int m_tickCount{};
};