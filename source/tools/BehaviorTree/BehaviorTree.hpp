#pragma once

#include "SequenceNode.hpp"
#include "SelectNode.hpp"
#include "InvertNode.hpp"
#include "RepeatNode.hpp"
#include "ActionNode.hpp"

#include <memory>

namespace cse498
{
    class BehaviorTree
    {
    public:
        BehaviorTree(std::unique_ptr<Node> root) : m_root(std::move(root)) { assert(m_root != nullptr); }

        int tick();
        int tickCount() const { return m_tickCount; }

        std::string getActivePath(); // Searching
        void debugView() const; // Traversal

    private:
        int m_tickCount{};
        std::unique_ptr<Node> m_root;
    };
}