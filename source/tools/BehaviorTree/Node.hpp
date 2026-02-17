#pragma once

#include <memory>
#include <vector>
#include <cassert>

class Node
{
public:
    std::string m_name;

    Node(std::string name)
        : m_name(std::move(name)) {}

    virtual ~Node() = default;

    virtual void print(int depth) const = 0;

    virtual std::string getActivePath() = 0;

    virtual int tick() = 0;

private:
};