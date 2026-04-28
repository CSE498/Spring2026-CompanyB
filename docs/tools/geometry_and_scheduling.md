# Geometry And Scheduling Tools

The Group 13 branch documents several support classes under `source/tools`. The infectious model mainly depends on the geometry tools, while scheduler changes are covered because they are part of the branch and have expanded tests.

## Point

Source: `source/tools/Point.hpp`

`Point` represents a continuous two-dimensional coordinate. The infectious model uses points as the bridge between discrete grid cells and continuous geometry checks.

`InfectiousWorld` converts a `WorldPosition` to a `Point` at the center of the grid cell before registering or querying shapes.

## Circle

Source: `source/tools/Circle.hpp`

`Circle` represents a center point and non-negative radius. It supports containment checks, overlap checks, distance calculations, intersection calculations, and transformations.

In the infectious model, each agent is represented as a zero-radius circle. Infection spread queries use circles centered on infected agents with radius `infection_radius`.

## Box

Source: `source/tools/Box.hpp`

`Box` is an axis-aligned rectangle stored by center and half dimensions. It supports construction from corners, containment checks, overlap checks, transforms, and geometry helpers.

In the infectious model, quarantine and treatment areas are stored as `Box` objects. `InfectiousWorld::AddQuarantineZone()` registers each box with the surface index.

## Surface

Source: `source/tools/Surface.hpp`

`Surface` is a spatial registry for `Circle` and `Box` shapes. It assigns stable shape IDs, stores shapes in a broad-phase sector grid, and performs exact overlap checks for circle-circle, box-box, and circle-box pairs.

`InfectiousWorld` uses `Surface` for two jobs:

- Registering agent circles so infection spread can query nearby agents.
- Registering quarantine boxes so movement and treatment logic can ask whether a position is inside quarantine.

The important surface operations for the model are:

- `AddCircle()`
- `AddBox()`
- `UpdateCircle()`
- `RemoveShape()`
- `QueryRadius()`

## WorldPath

Source: `source/tools/WorldPath.hpp`

`WorldPath` stores ordered `Point` values and exposes path operations such as total length, segment length, interpolation, reversal, validation, and intersection-style checks. It is not the main disease state container, but it is part of the branch's movement and geometry support surface.

## Scheduler

Source: `source/tools/Scheduler.hpp`

`Scheduler` is a generic priority scheduler. It supports deterministic weighted round-robin and probabilistic weighted selection. It also includes optional rebalancing and failure/backoff behavior.

The scheduler is not directly required for `InfectiousWorld` infection spread, but it is part of the branch's shared tool work and is covered by `tests/tools/SchedulerTest.cpp`.

