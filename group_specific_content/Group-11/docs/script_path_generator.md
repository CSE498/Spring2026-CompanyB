## `PathGenerator`

### Introduction

Hey everyone, I'm Chirag. I worked on `PathGenerator`: it's the class that computes routes for agents and hands them back as `WorldPath` objects.

### Broad overview

The idea is to centralize all navigation logic in one place so agents don't have to figure out their own routes — they just ask for a path and follow it. `PathGenerator` supports several path types: A\* shortest paths, patrol loops through waypoints, avoidance paths that route around a region, random walks for wandering behavior, and expanding spirals for search patterns.

The world interface is kept abstract through a `WorldQueryFunc` callback — a simple `bool(const Point&)` that answers "can I move here?" That means `PathGenerator` doesn't care whether you're on a grid, a continuous field, or a physics simulation. You plug in whatever predicate makes sense for your world.

### Usage example

```cpp
#include "src/PathGenerator.hpp"
#include <iostream>

int main() {
  cse498::PathGenerator gen;

  // Open world — everything is walkable
  auto openWorld = [](const cse498::Point &) { return true; };

  // A* shortest path
  auto path = gen.ShortestPath({0, 0}, {5, 5}, openWorld);
  if (path) {
    std::cout << "Shortest path points: " << path->size() << "\n";
    std::cout << "Total length: " << path->totalLength() << "\n";
  }

  // Patrol loop through three waypoints
  auto patrol = gen.PatrolPath({{0,0}, {4,0}, {4,4}}, /*loop=*/true);
  if (patrol)
    std::cout << "Patrol path points: " << patrol->size() << "\n";

  // Avoidance — route from (0,0) to (6,0), stay 2 units away from (3,0)
  auto avoid = gen.AvoidancePath({0,0}, {6,0}, {3,0}, 2.0, openWorld);
  if (avoid)
    std::cout << "Avoidance path found, length: " << avoid->totalLength() << "\n";
  else
    std::cout << "No avoidance path found\n";

  return 0;
}
```

`ShortestPath` runs A\* and returns an `std::optional<WorldPath>` — if no route exists you get `nullopt`, otherwise a ready-to-use path. `PatrolPath` stitches waypoints together with A\* segments and, with `loop=true`, closes the circuit back to the start. `AvoidancePath` wraps the same A\* but with an extra predicate that rejects any point inside the avoidance radius — so you just describe the zone to stay out of and the algorithm figures out the detour.

### Error handling

Two tiers. Programmer bugs — null `WorldQueryFunc`, negative avoidance radius, non-positive step size — those assert. You shouldn't hit them in normal use.

Runtime edge cases return something useful: if no path exists between start and goal (blocked world, avoidance zone cuts off all routes, search exceeds the 10000 iteration cap), you get `std::nullopt`. `RandomWalk` never fails — if it hits a dead end it just returns whatever partial path it managed to build. `PatrolPath` silently skips unreachable waypoints and connects what it can.

### Limitations & restrictions

A\* here is 8-directional and grid-stepped — diagonal moves cost the same as cardinal, which is fine for agents but isn't true continuous pathfinding. The 10000 iteration cap prevents runaway searches on large maps but means very long paths may return `nullopt` even when a route technically exists; lower the step size only when you need fine-grained paths and the world is small. `SpiralPath` and `RandomWalk` ignore the `canMove` callback for the start point validation in the spiral case — those are purely geometric generators. Path smoothing is not implemented, so returned paths will follow the step grid rather than cutting corners.
