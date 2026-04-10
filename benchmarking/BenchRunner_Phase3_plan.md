# Phase 3 Plan: Core Implementation (Benchmarking)

## Objective

Implement benchmark runtime behavior using frozen Phase 2 interfaces, with no contract drift.

Primary alignment target: [benchmarking/benchmarking_spec.md](benchmarking/benchmarking_spec.md)
Contract baseline: [benchmarking/BenchRunner_Phase2_spec.md](benchmarking/BenchRunner_Phase2_spec.md)

## Phase 3 Implementation Checklist

### Checklist 1 - Freeze the remaining implementation contract

- [ ] Keep [benchmarking/BenchmarkTypes.hpp](benchmarking/BenchmarkTypes.hpp) as the canonical naming source for all runtime code.
- [ ] Keep Params required-only and do not reintroduce optional fields.
- [ ] Keep MetricSample names exactly as currently defined: `wall_time_ns`, `memory_usage_kb`, `success`, `error_message`.
- [ ] Keep BenchmarkResult names exactly as currently defined: `avg_wall_time_ns`, `min_wall_time_ns`, `max_wall_time_ns`, `stdev_wall_time_ns`, `avg_memory_usage_kb`, `min_memory_usage_kb`, `max_memory_usage_kb`, `stdev_memory_usage_kb`, `sample_count`.

### Checklist 2 - Implement BenchRunner registration and lookup

- [ ] Keep `BenchmarkCallable = std::function<void(Params&)>` in [benchmarking/BenchRunner.hpp](benchmarking/BenchRunner.hpp).
- [ ] Preserve replace-on-duplicate registration behavior in [benchmarking/BenchRunner.cpp](benchmarking/BenchRunner.cpp).
- [ ] Keep deterministic ordering through the ordered registry.
- [ ] Make `HasBenchmark`, `Size`, and `ListBenchmarkIds` behave as stable query APIs.

### Checklist 3 - Implement MetricCollector lifecycle

- [ ] Define the active measurement lifecycle in [benchmarking/MetricCollector.cpp](benchmarking/MetricCollector.cpp).
- [ ] Make `BENCH_START()` begin a fresh measurement window.
- [ ] Make `BENCH_STOP()` close the current measurement window and finalize the sample.
- [ ] Make `GetSample()` return the finalized MetricSample for the last completed measurement.
- [ ] Return `TimingNotStarted` when stop is called out of order.
- [ ] Return `MemoryReadFailure` when RSS capture fails.

### Checklist 4 - Implement timed execution flow

- [ ] Validate `Params` before execution and reject invalid runs early.
- [ ] Execute one untimed warmup block before measured repetitions.
- [ ] Run the benchmark callable once per measured repetition.
- [ ] Treat callable exceptions as `BenchmarkFailure`.
- [ ] Return `RunStatus::Ok` only when the measured benchmark run completes successfully.

### Checklist 5 - Implement result aggregation

- [ ] Collect one MetricSample per measured repetition.
- [ ] Exclude invalid samples from aggregate math.
- [ ] Compute average, min, max, and standard deviation for wall time.
- [ ] Compute average, min, max, and standard deviation for memory usage.
- [ ] Set `sample_count` to the number of successful samples used.
- [ ] Keep all timing and memory aggregates in `std::uint64_t` form.

### Checklist 6 - Keep error handling deterministic

- [ ] Return `UnknownBenchmark` when the requested benchmark id is missing.
- [ ] Return `InvalidParams` when input validation fails.
- [ ] Return `BenchmarkFailure` on callable exceptions or benchmark execution faults.
- [ ] Keep error outcomes explicit instead of partially succeeding.

### Checklist 7 - Verify behavior against the spec

- [ ] Confirm registration still matches the registry pattern in [benchmarking/benchmarking_spec.md](benchmarking/benchmarking_spec.md).
- [ ] Confirm warmup remains untimed and measured repetitions are averaged.
- [ ] Confirm wall time is measured in nanoseconds.
- [ ] Confirm memory is captured as RSS delta behavior.
- [ ] Confirm the implementation still aligns with the plug-in interface design in the benchmark spec.

### Checklist 8 - Validate the implementation

- [ ] Check that compile diagnostics are clean for the benchmark files.
- [ ] Check that registration replacement behaves as expected.
- [ ] Check that invalid Params are rejected.
- [ ] Check that stop-before-start returns the correct MetricCollector error.
- [ ] Check that a successful run returns `RunStatus::Ok`.
- [ ] Check that aggregates populate the canonical result fields.

### Checklist 9 - Complete Phase 3

- [ ] Confirm BenchRunner and MetricCollector runtime logic are implemented.
- [ ] Confirm warmup and repetition behavior matches the benchmark spec.
- [ ] Confirm aggregation writes into the canonical benchmark result shape.
- [ ] Confirm errors return stable typed statuses.
- [ ] Confirm the implementation can be handed off to reporting and build integration work.
