# Phase 2 Plan: Interface Freeze (Benchmarking)

## Objective

Freeze concrete C++ type contracts and header signatures before adding new runtime logic.

Primary alignment target: [benchmarking/benchmarking_spec.md](benchmarking/benchmarking_spec.md)

## Scope

In scope for Phase 2:

- Header-only contract finalization in:
  - [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)
  - [benchmarking/BenchRunner.hpp](benchmarking/BenchRunner.hpp)
  - [benchmarking/MetricCollector.hpp](benchmarking/MetricCollector.hpp)
- Explicit result and metric sample schemas
- Callable and status semantics (registration, validation, failure categories)

Out of scope for Phase 2:

- Full experiment-loop and tick-loop behavior implementation
- Report export implementation details
- Build integration details

## Source Alignment Matrix

| Spec Requirement                                     | Contract Surface                                                                                                                         | Freeze Decision                                                    |
| ---------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| Registry pattern with `std::function<void(Params&)>` | [benchmarking/BenchRunner.hpp](benchmarking/BenchRunner.hpp)                                                                             | Keep `BenchmarkCallable = std::function<void(Params&)>`            |
| Parameterized runs using `Params`                    | [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)                                                                       | Keep canonical `Params` in shared types header                     |
| Warmup + repetitions supported                       | [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)                                                                       | `warmup_ticks`, `repetitions`, `tick_count` remain required fields |
| Timing and RSS metrics                               | [benchmarking/MetricCollector.hpp](benchmarking/MetricCollector.hpp), [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp) | `MetricSample` must include wall time and RSS delta fields         |
| Aggregation/report handoff                           | [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)                                                                       | `BenchmarkResult` must include per-point aggregate metrics         |

## Frozen Type Contracts

### 1) Params (shared)

Location: [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)

Freeze decisions:

- Required fields: `benchmark_name`, `agent_count`, `tick_count`, `warmup_ticks`, `repetitions`, `seed`
- Optional fields: none (intentionally removed)
- Validation contract: `IsValid()` must reject zero/empty invalid values

### 2) MetricSample (shared)

Location: [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)

Freeze decisions:

- Required fields:
  - `wall_time_ns`
  - `memory_usage_kb`
  - `success`
  - `error_message`

### 3) BenchmarkResult (shared)

Location: [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp)

Freeze decisions:

- Required fields:
  - `agent_count`
  - `avg_wall_time_ns`
  - `min_wall_time_ns`
  - `max_wall_time_ns`
  - `stdev_wall_time_ns`
  - `avg_memory_usage_kb`
  - `min_memory_usage_kb`
  - `max_memory_usage_kb`
  - `stdev_memory_usage_kb`
  - `sample_count`

### 4) BenchRunner public API

Location: [benchmarking/BenchRunner.hpp](benchmarking/BenchRunner.hpp)

Freeze decisions:

- `Register(id, callable)` stays replace-on-duplicate policy
- `HasBenchmark(id)` remains const query API
- `Size()` and `ListBenchmarkIds()` remain deterministic with ordered registry
- `RunBenchmarkById(id, params)` remains the single-run contract for early execution validation

### 5) MetricCollector public API

Location: [benchmarking/MetricCollector.hpp](benchmarking/MetricCollector.hpp)

Freeze decisions:

- Expose start/stop timing interface equivalent to `BENCH_START()` / `BENCH_STOP()` behavior
- Return typed success/error category using `MetricCollectorError`
- Keep memory collection implementation hidden behind API

## Current Gaps To Close Before Phase 2 Exit

1. Document the finalized naming vocabulary as canonical for Phase 3 implementation (no alias names required).
2. Ensure `MetricCollector` ownership/lifecycle semantics are explicit for `BENCH_START()` -> `BENCH_STOP()` -> `GetSample()` usage.
3. Confirm all benchmark timing fields stay `std::uint64_t` end-to-end in implementation.
4. Keep report/aggregator field names identical to [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp).

## Exit Criteria (Phase 2 Complete)

- All public benchmark interfaces are declared in headers and compile cleanly.
- Type contracts for params, sample, and aggregate result are fully concrete.
- No unresolved TODO placeholders in benchmark headers.
- API behavior ambiguity is removed for registration, run status, and metrics capture boundaries.
