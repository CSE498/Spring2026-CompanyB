# Group 13 Model Context

Group 13 implements a step-based infectious disease simulation. The central class is `cse498::InfectiousWorld` in `source/Worlds/InfectiousWorld.hpp`. It inherits from `SimWorldBase<DiseaseData>`, which inherits from `StepWorldBase<DiseaseData>`.

The model uses `DiseaseData` from `source/core/AgentData.hpp` as each agent's state. `DiseaseData` stores `WorldPosition position`, `HealthState health`, `size_t ticks_in_state`, `size_t quarantine_ticks`, and optional `WorldPosition destination`. `HealthState` has `SUSCEPTIBLE`, `INFECTED`, and `RECOVERED`.

Each simulation tick has two phases. First, `RunAgents()` asks every `StepAgentBase<DiseaseData>` for a `StepContainer` and calls `InfectiousWorld::DoAction()` to apply movement and info steps. Second, `InfectiousWorld::UpdateWorld()` increments `tick_count`, updates health timers, spreads infection, and calls an optional observer.

Movement is grid-based. An agent can move only if the target cell is valid, is not a wall, is not occupied by another agent, and does not violate quarantine rules. Susceptible and recovered agents cannot enter quarantine zones. Infected agents cannot leave quarantine zones until treatment recovers them.

Infection spread is proximity-based. The world registers each agent as a zero-radius `Circle` in a `Surface` spatial index. Infected agents query nearby shapes using `Surface::QueryRadius()` and may infect susceptible agents within `infection_radius` according to `transmission_rate`.

Quarantine zones are `Box` objects registered with the same `Surface`. `AddQuarantineZone()` stores the box and its surface ID. `IsInQuarantine()` checks whether a grid position overlaps a registered quarantine shape.

Recovery can happen in two ways. If an infected agent stays in quarantine for `treatment_duration` ticks, it becomes recovered. If `fallback_recovery_ticks` is greater than zero, infected agents outside treatment recover after that many infected ticks. Recovered agents can return to susceptible if `immunity_duration` is greater than zero.

The main agents are `ScriptedAgent`, `SwarmingAgent`, and `StepPacingAgent`. `ScriptedAgent` cycles down, right, up, left. `SwarmingAgent<DiseaseData>` chooses random neighboring movement while disease transitions remain in the world. `StepPacingAgent` paces along a row or column, but follows a BFS route when infected or recovered and given a destination.

The core supporting tools are `Point`, `Circle`, `Box`, and `Surface`. `Point` represents continuous 2D coordinates. `Circle` supports radius and overlap checks. `Box` represents axis-aligned rectangles. `Surface` stores circles and boxes in spatial sectors and performs broad-phase plus exact overlap checks.

The primary behavioral tests are in `tests/InfectiousWorldTest.cpp`. They cover initial counts, manual infection, proximity spread, quarantine treatment recovery, fallback recovery, immunity expiration, and reinfection after immunity.

