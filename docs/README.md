# Group 13 Branch Documentation

This documentation covers the Group 13 model code currently on the `Group-13` branch. It intentionally lives outside `group_specific_content` so it can be folded into the shared company documentation tree later.

## Files

- [AI model context](ai/group13_model_context.md): compact context for loading the Group 13 model into an LLM.
- [Infectious world](worlds/infectious_world.md): main disease simulation world, health lifecycle, movement rules, and quarantine behavior.
- [Agents](agents/group13_agents.md): step-based agents used by the infectious model.
- [Core model data and step flow](core/step_model_data.md): shared state objects and turn execution model.
- [Geometry and scheduling tools](tools/geometry_and_scheduling.md): Group 13 support classes used by the model and related tests.

## Scope

The main model is a grid-based infectious disease simulation implemented by `InfectiousWorld`. Agents expose turns as `StepContainer` objects. The world validates those steps against the grid, quarantine rules, and occupancy, then updates disease state after all agents have moved.

The branch also documents supporting geometry classes such as `Point`, `Box`, `Circle`, and `Surface`, because the infectious model depends on them for infection radius checks and quarantine zone queries.

