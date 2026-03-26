#pragma once

#include "CompositeNode.hpp"

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Composite node that selects the first successful child.
 *
 * A SelectNode (also known as a Selector or Fallback node)
 * ticks its children in order from left to right until:
 *
 * - A child returns Status::Success -> the selector immediately returns
 * Status::Success.
 * - A child returns Status::Running -> the selector returns Status::Running and
 * resumes from that child on the next tick.
 * - All children return Status::Failure -> the selector returns
 * Status::Failure.
 *
 * The node maintains internal state to remember which child
 * is currently active between ticks.
 */
class SelectNode : public CompositeNode {
 public:
  using CompositeNode::CompositeNode;

  Status tick(Blackboard& blackboard) override {
    ++m_tickCount;

    bool isRunning = false;

    // Check if children are empty
    auto& children = this->getChildren();
    if (children.empty()) return Status::Running;

    // Check status of active child (Pick last child if m_index is out of
    // bounds)
    auto& child = (*(children.begin() + m_index))
                      ? *(children.begin() + m_index)
                      : *(children.end() - 1);
    Status status = child->tick(blackboard);

    // Short Circuit if m_status is Status::Success or Status::Failure
    if (m_status == Status::Success || m_status == Status::Failure)
      return m_status;

    switch (status) {
      case Status::Success:
        m_status = Status::Success;
        return Status::Success;
        break;

      case Status::Running:
        isRunning = true;
        break;

      case Status::Failure:
        ++m_index;
        isRunning = true;
        break;
    }

    if (isRunning && m_index < children.size()) {
      m_status = Status::Running;
    }

    else {
      m_status = Status::Failure;
      --m_index;
    }
    return m_status;
  };

  int tickCount() const { return m_tickCount; }

  std::string getActivePath() override {
    auto& children = this->getChildren();
    if (children.empty()) return m_name;

    auto& child = (*(children.begin() + m_index))
                      ? *(children.begin() + m_index)
                      : *(children.end() - 1);
    return m_name + " - " + child->getActivePath();
  };

 private:
  /// Tracks the total number of tick() calls made on this node.
  int m_tickCount{};

  /// Index of the currently active child node.
  unsigned int m_index{};
};