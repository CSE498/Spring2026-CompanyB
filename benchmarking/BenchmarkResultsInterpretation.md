# Benchmark Results Interpretation Guide

## Why this guide exists

Benchmark reports are only useful if everyone reads them the same way.
This guide explains what each metric means and how to draw safe conclusions.

## What each result tells you

### Time metrics (nanoseconds)

- avg_wall_time_ns: Typical cost per measured run.
- min_wall_time_ns: Best observed run; often close to ideal conditions.
- max_wall_time_ns: Worst observed run; useful for stability checks.
- stdev_wall_time_ns: Run-to-run variability in timing.

Interpretation:

- Lower average is better for throughput and latency.
- Large spread between min and max means unstable performance.
- High standard deviation means results are noisy and may need more repetitions.

### Memory metrics (KB)

- avg_memory_usage_kb: Typical memory delta captured during measured runs.
- min_memory_usage_kb and max_memory_usage_kb: Best and worst observed memory behavior.
- stdev_memory_usage_kb: Variability in memory usage.

Interpretation:

- Rising average with larger agent_count is expected, but slope matters.
- High max relative to average may indicate spikes or temporary allocations.
- High standard deviation suggests inconsistent allocation patterns.

### RSS-at-stop metrics (KB)

- avg_current_rss_at_stop_kb: Typical process RSS at BENCH_STOP for measured runs.
- min/max/stdev_current_rss_at_stop_kb: Distribution of resident memory footprint at stop points.

Interpretation:

- Useful for tracking retained memory over repeated runs.
- If this climbs across comparable experiments, investigate retention or caching behavior.

## How to judge scaling

When you increase agent_count or tick_count:

- Near-linear growth means scaling is generally healthy.
- Faster-than-linear growth is a warning sign for algorithmic bottlenecks.
- Flat or inconsistent trend lines can indicate measurement noise or benchmark setup issues.

## How to compare two benchmark runs safely

Only compare runs when all of these are unchanged:

- same benchmark_name and benchmark callable logic
- same tick_count and warmup_ticks
- same repetitions and similar system load
- same build mode (Release benchmark mode)

If these differ, treat the comparison as directional only, not definitive.

## Status and failure signals

### RegistrationResult

- Inserted: benchmark id was added.
- Replaced: existing benchmark id was overwritten.
- InvalidId: benchmark id was empty.
- InvalidCallable: benchmark function was empty.

### RunStatus

- Ok: run completed and aggregation succeeded.
- UnknownBenchmark: id was not registered.
- InvalidParams: Params failed validation.
- BenchmarkFailure: benchmark body reported failure.
- MetricFailure: metric collection failed.

### RunAndReportStatus

- Ok: run and report write succeeded.
- RunFailed: benchmark execution failed.
- MissingResult: no aggregate result available after run.
- InvalidOutputFileName: output name failed validation.
- DirectoryCreateFailed: output directory could not be created.
- ReportWriteFailed: report generation failed.

## Practical decision checklist

Before you trust a benchmark report:

- Confirm sample_count equals repetitions.
- Confirm stdev is not dominating the average.
- Confirm no failure statuses occurred.
- Confirm run configuration is documented beside the output.

If the checklist fails, rerun with tighter controls before drawing conclusions.
