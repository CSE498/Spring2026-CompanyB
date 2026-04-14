# Group Benchmark Files

All group benchmark mains live in this folder.

## File placement

- Put benchmark entry files in this folder.
- Use descriptive names, for example: `group12_string_benchmark.cpp`.
- See `example_1.cpp` and `example_2.cpp` for simple reference implementations.

## Run a benchmark

From repo root:

```bash
make benchmark BENCH_FILE=group12_string_benchmark.cpp
```

Or with full path:

```bash
make benchmark BENCH_FILE=groups/group12_string_benchmark.cpp
```

## Benchmark flow

All benchmarks follow this pattern:

1. Create a `BenchRunner` instance
2. Register your benchmark function with `runner.Register(id, lambda_function)`
3. Set up `Params` with your desired configuration
4. Call `runner.RunBenchmarkAndWriteReport(id, params, output_file_name)`

The `RunBenchmarkAndWriteReport` method handles:

- Running your registered benchmark
- Capturing timing and memory metrics
- Aggregating results across repetitions
- Writing the report to `benchmarking/groups/results/<output_file_name>`

See `example_1.cpp` for a complete working example.
