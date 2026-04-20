# Group 13 Agents

The Group 13 branch uses step-based agents that return `StepContainer` objects. The world decides whether each requested step is valid and then writes the updated state back to the agent.

## Shared Agent Contract

All model agents derive from `StepAgentBase<DataClass>`. The base class stores a stable agent ID and a copy of the agent's current data object.

Important methods:

- `GetId()` returns the stable world-assigned ID.
- `GetTurn()` returns the next `StepContainer`.
- `GetState()` returns the current data copy.
- `SetState()` replaces the current data after the world applies a turn.
- `SetGoal(WorldPosition)` gives agents a common goal-setting hook.

## ScriptedAgent

Source: `source/Agents/ScriptedAgent.hpp`

`ScriptedAgent<DataClass>` is a deterministic baseline agent. It cycles through a fixed movement pattern:

1. Down
2. Right
3. Up
4. Left

It works with data classes that expose either `position` or `pos`. This makes it useful for simple tests and demos because its turn sequence is predictable.

`SetGoal()` is intentionally ignored. The agent does not path toward a caller-provided destination.

## SwarmingAgent

Source: `source/Agents/SwarmingAgent.hpp`

`SwarmingAgent<SwarmData>` supports `TrafficData` and `DiseaseData`.

For `DiseaseData`, it chooses a random cardinal neighbor each turn. Infection state changes are not handled by the agent; they remain centralized in `InfectiousWorld`.

For `TrafficData`, it can move toward an optional destination. It keeps a small recent-position history so it can avoid immediate loops and choose a backup movement if the primary movement is blocked.

Important behavior:

- Recent movement history uses a fixed size of six positions.
- Random movement prefers positions that are not in recent history.
- Traffic movement can use an `InfoStep` and `ConditionalStep` to ask the world whether a preferred target is available.
- `SetGoal()` writes to the data object's `destination` field when that field exists.

## StepPacingAgent

Source: `source/Agents/dummyStepPacingAgent.hpp`

`StepPacingAgent` is a `DiseaseData` agent that normally walks back and forth along a row or column. If its previous requested movement did not change its position, it reverses direction.

When the agent has a destination and is infected or recovered, it switches into BFS navigation. The BFS uses a caller-provided passability predicate from `SetPassable()`.

Important behavior:

- `SetHorizontal()` makes the agent pace left and right.
- `SetVertical()` makes the agent pace up and down.
- `SetPassable()` installs the walkability check used by BFS.
- BFS recomputes when the destination changes, the path is exhausted, or a previously requested BFS step was blocked.

This lets infected agents navigate toward treatment and recovered agents navigate toward an exit while still supporting simple pacing behavior when no disease-driven destination is active.

