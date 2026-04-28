# Infectious World

`InfectiousWorld` is the Group 13 disease simulation world. It lives in `source/Worlds/InfectiousWorld.hpp` and models infectious spread over a discrete `WorldGrid`.

## Role In The Model

`InfectiousWorld` owns the grid, the disease agents, quarantine zones, and disease parameters. It is responsible for applying agent movement, enforcing world constraints, updating health timers, and spreading infection.

The class inherits from `SimWorldBase<DiseaseData>`. `SimWorldBase` provides common grid simulation helpers such as cell type IDs, tick count, grid access, agent lookup, and simple spatial queries.

## Tick Flow

The world follows the `StepWorldBase` execution pattern:

1. `RunAgents()` loops over agents.
2. Each agent returns a `StepContainer` from `GetTurn()`.
3. `InfectiousWorld::DoAction()` consumes the steps and returns updated `DiseaseData`.
4. `UpdateWorld()` runs once after all agent turns.

`UpdateWorld()` increments the tick count, calls `UpdateHealthTimers()`, calls `SpreadInfection()`, and then invokes the optional tick observer.

The timer-before-spread ordering is important. Agents infected during the current `UpdateWorld()` call begin with `ticks_in_state == 0` and do not advance their infection timer until the next tick.

## Movement Rules

`DoAction()` applies `MovementStep` requests if all of these are true:

- The target position is inside the grid.
- The target cell is not a wall.
- No other agent already occupies the target cell.
- The target does not violate quarantine rules.

Quarantine movement rules:

- Susceptible and recovered agents cannot enter quarantine zones.
- Infected agents can enter quarantine zones.
- Infected agents cannot leave quarantine zones until treatment recovers them.

The same availability checks are used for `InfoStep::Aspect::LOC_AVAIL`, which allows conditional agents to ask whether a location can be used before committing to a movement step.

## Disease State

Each agent carries `DiseaseData`:

- `position`: current grid location.
- `health`: `SUSCEPTIBLE`, `INFECTED`, or `RECOVERED`.
- `ticks_in_state`: number of ticks spent in the current health state.
- `quarantine_ticks`: number of infected ticks spent inside treatment.
- `destination`: optional target such as clinic entrance or recovery exit.

The world exposes count helpers for each state: `GetSusceptibleCount()`, `GetInfectedCount()`, and `GetRecoveredCount()`.

## Infection Spread

Infection spread is handled by `SpreadInfection()`.

Each agent is represented inside a `Surface` as a zero-radius `Circle` located at the center of its grid cell. Before spread, the world synchronizes those circles to each agent's current position.

For each infected agent, the world queries the surface using `infection_radius`. Nearby quarantine shapes are ignored, and nearby susceptible agents can become infected if a random roll is below `transmission_rate`.

Newly infected agents are updated after the spread pass completes. This prevents order-dependent chain infection within a single spread loop.

## Quarantine And Recovery

Quarantine zones are stored as `Box` objects and registered with `Surface`. `IsInQuarantine()` checks whether a world position overlaps one of those registered quarantine shapes.

Recovery paths:

- Treatment recovery: infected agents inside quarantine recover after `treatment_duration` quarantine ticks.
- Fallback recovery: if `fallback_recovery_ticks > 0`, infected agents outside treatment recover after that many infected ticks.
- Immunity expiration: if `immunity_duration > 0`, recovered agents become susceptible again after that many recovered ticks.

The world can assign infected agents a clinic destination with `SetClinicEntrance()`. It can also assign recovered agents an exit destination with `SetRecoveryExit()`.

## Public Configuration

Important configuration methods:

- `SetTransmissionRate(double rate)`
- `SetInfectionRadius(double r)`
- `SetTreatmentDuration(size_t ticks)`
- `SetFallbackRecoveryTicks(size_t ticks)`
- `SetImmunityDuration(size_t ticks)`
- `SetClinicEntrance(WorldPosition pos)`
- `SetRecoveryExit(WorldPosition pos)`
- `RegisterTickObserver(std::function<void(const InfectiousWorld&)>)`

Important world editing methods:

- `AddQuarantineZone(const Box& zone)`
- `ClearQuarantineZones()`
- `InfectAgent(size_t id)`

## Tests

The main behavior tests are in `tests/InfectiousWorldTest.cpp`. They verify empty counts, manual infection, invalid infection IDs, proximity spread, quarantine recovery, fallback recovery, immunity expiration, and reinfection after immunity.

