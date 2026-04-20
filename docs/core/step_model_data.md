# Step Model Data And Execution

The Group 13 model uses the shared step system instead of letting agents directly mutate the world. Agents request actions, and the world decides what actually happens.

## Data Classes

Source: `source/core/AgentData.hpp`

`AgentData.hpp` defines lightweight state objects for step-based worlds.

`TrafficData` is used by traffic-style swarming behavior. It stores a current position, optional destination, direction, active flag, display symbol, and display color.

`DiseaseData` is used by `InfectiousWorld`. It stores:

- `WorldPosition position`
- `HealthState health`
- `size_t ticks_in_state`
- `size_t quarantine_ticks`
- `std::optional<WorldPosition> destination`

`HealthState` has three values:

- `SUSCEPTIBLE`
- `INFECTED`
- `RECOVERED`

The branch also contains `source/core/DiseaseData.hpp`, but the active infectious model includes `AgentData.hpp` and uses the `DiseaseData` definition from there.

## StepAgentBase

Source: `source/core/StepAgentBase.hpp`

`StepAgentBase<DataClass>` is the base class for agents that participate in the step model. It owns an ID and a data object.

The important design decision is that `GetState()` returns a copy and `SetState()` replaces the state. This keeps state changes flowing through the world instead of letting agents directly edit global world data.

## StepWorldBase

Source: `source/core/StepWorldBase.hpp`

`StepWorldBase<DataClass>` owns the agent list and grid. It assigns IDs when agents are added and provides the default turn loop:

1. Ask an agent for a turn.
2. Pass the agent into `DoAction()`.
3. Store the returned data object back into the agent.

Derived worlds implement `DoAction()` because only the world knows which requested steps are valid.

## Step Containers

Source: `source/core/Step.hpp`

Agents return `StepContainer` objects. These containers can hold movement requests, information requests, conditional logic, and related step types. `InfectiousWorld` currently handles `MovementStep` and `InfoStep::Aspect::LOC_AVAIL` directly. Other step types are handled by `StepContainer` flow control or ignored by the infectious world if they are not relevant.

## Why The Model Uses This Pattern

The step system separates agent intent from world authority.

Agents decide what they want to try. The world checks grid bounds, walls, occupancy, quarantine rules, and disease rules. This keeps infection and movement policy consistent even when different agent types are used in the same world.

