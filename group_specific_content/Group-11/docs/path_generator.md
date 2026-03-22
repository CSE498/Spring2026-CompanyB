# Class: PathGenerator (Chirag Bhansali)

## 1) Class Description

`PathGenerator` takes information about a simulated world and creates `WorldPath` objects with specific properties. Instead of making agents figure out their own routes, we centralize the pathfinding logic here so it's reusable across different world types and agent behaviors.

The main goal is to provide a clean interface where you request a path type (shortest route, patrol loop, avoidance path, etc.) and get back a ready-to-use `WorldPath`. This separates navigation logic from agent behavior-agents just need to follow the path, not compute it. The generator needs to work with different world representations so we're keeping the world query interface abstract.

## 2) Similar Standard Library Classes

- `std::function`: We'll use this to pass in world query callbacks (like "is this point walkable?" or "what's the cost to move here?") so PathGenerator doesn't need to know about specific World implementations.
- `std::priority_queue` (C++17): For A\* and Dijkstra pathfinding algorithms, we need to efficiently get the lowest-cost node.
- `std::unordered_map`: To track visited nodes and reconstruct paths after searching.
- `std::optional` (C++17): Some path requests might be impossible (no route exists), so we return `std::optional<WorldPath>` instead of throwing exceptions.

## 3) Key Functions

### Core Path Generation

- `std::optional<WorldPath> ShortestPath(Point start, Point goal, WorldQueryFunc canMove);` - A\* pathfinding for the optimal route.
- `std::optional<WorldPath> PatrolPath(const std::vector<Point>& waypoints, bool loop = true);` - Connects waypoints in order, optionally returning to start.
- `std::optional<WorldPath> AvoidancePath(Point start, Point goal, Point avoid, double radius, WorldQueryFunc canMove);` - Routes from A to B while staying away from C.

### Utility Generation

- `WorldPath RandomWalk(Point start, size_t steps, WorldQueryFunc canMove);` - Generates a wandering path for exploration behaviors.
- `WorldPath SpiralPath(Point center, double spacing, size_t turns);` - Creates expanding spiral patterns (useful for search behaviors).

### Configuration

- `void SetHeuristic(HeuristicFunc h);` - Allows switching between Manhattan, Euclidean, or custom distance metrics.
- `void SetStepSize(double size);` - Controls granularity for continuous worlds.

## 4) Error Conditions

### (1) Programmer Error - Assert

- Passing empty waypoint lists to `PatrolPath`.
- Setting negative `radius` in `AvoidancePath`.
- Providing null `WorldQueryFunc` callbacks.

### (2) Recoverable Error - Optional

- No path exists between start and goal → returns `std::nullopt`.
- Avoidance constraint makes path impossible → returns `std::nullopt`.
- Timeout on long searches (if we implement iteration limits) → returns `std::nullopt`.

### (3) User-Level / Soft Errors - Return Condition

- `RandomWalk` hitting a dead-end just returns the partial path generated so far.
- `PatrolPath` with unreachable waypoints skips those points and connects what it can.

## 5) Expected Challenges

- **World Abstraction**: Different worlds (grid vs continuous, 2D vs 3D) need different pathfinding approaches. We'll need a clean callback interface so we don't hardcode world types.
- **Performance**: A\* on large continuous spaces can be slow. We might need to implement path smoothing or hierarchical pathfinding for big maps.
- **Dynamic Worlds**: If obstacles move (agents, closing doors), cached paths become invalid. We might need to add a "path validation" function or coordinate with Group 20 to track when worlds change.
- **Edge Cases**: Paths along world boundaries, overlapping avoidance zones, or degenerate cases (start == goal) need careful handling.

## 6) Coordination with Other Groups

- **Group 11 (Internal)**: We create `WorldPath` objects, so we depend on their interface. `BehaviorTree` will likely call our functions as leaf actions.
- **Group 13 (Math World)**: Need their obstacle/collision detection API to check if paths are valid in physics simulations.
- **Group 19 (Interaction World)**: Need their grid/tile query API for pathfinding in structured environments.
- **Group 20 (Data Analytics)**: Should log which path types are requested most to help optimize our algorithms.
