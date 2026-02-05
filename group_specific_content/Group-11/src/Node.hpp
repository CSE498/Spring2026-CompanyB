#pragma once

#include <memory>
#include <ostream>

class Node
{
public:
    int tick() virtual = 0;

private:
    int id;
}