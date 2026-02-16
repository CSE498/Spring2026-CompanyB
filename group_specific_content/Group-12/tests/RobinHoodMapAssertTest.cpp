/**
 * @file RobinHoodMapAssertTest.cpp
 * 
 * @brief Negative compilation test for RobinHoodMap's static_assert.
 * This file is expected to fail compilation. The static_assert in
 * RobinHoodMap requires K to be default constructible instantiating
 * with a non-default-constructible key type must produce a compile error
 *
 * @author John Stouffer
 * @date 2026-2-16
 */

#include "../RobinHoodMap.hpp"

struct NonDefaultConstructible {
    NonDefaultConstructible(int x) : val(x) {}
    int val;
};

RobinHoodMap<NonDefaultConstructible, int> map;

int main() { return 0; }
