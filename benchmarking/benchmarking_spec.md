# Project Context: C++ Performance Engineering Suite
**Target Project:** Company B / Group 12 (Swarming Agents) - MSU Campus Simulation
**Objective:** Build a modular "plug-in" benchmarking and regression suite to measure simulation scalability (specifically Agent Count vs. Runtime).

## 1. Technical Constraints
* **Language:** Modern C++ (C++20/23).
* **Environment:** Linux/Docker (using `getrusage` for memory tracking).
* **Performance Goal:** High-precision timing with `std::chrono::high_resolution_clock`.
* **Architecture:** Modular, "Plug-in" design using the **Registry Pattern**.

## 2. Core Modules to Implement

### Module A: `BenchRunner` (The Orchestrator)
* **Functionality:** Manages the "Experiment Loop" (hyperparameters) and "Execution Loop" (ticks).
* **Key Pattern:** Uses a `std::function<void(Params&)>` registration system.
* **Logic:** * Iterate through a range of `Params.agent_count`.
    * Run a "Warmup" phase (untimed).
    * Execute the user-provided lambda multiple times to get a statistical average.

### Module B: `MetricCollector` (The Watcher)
* **Responsibility:** Capture "before and after" snapshots.
* **Metrics:** * **Wall Time:** Nanoseconds via `std::chrono`.
    * **Memory:** Resident Set Size (RSS) delta via `sys/resource.h` (`getrusage`).
* **Implementation:** Provide `START_TIMER()` and `STOP_TIMER()` macros or functions to be used inside test lambdas.

### Module C: `ReportGenerator` (The Exporter)
* **Responsibility:** Aggregate results and export to CSV/JSON.
* **Analysis:** Calculate average time per agent and detect $O(n)$ vs $O(n^2)$ scaling trends.

## 3. The "Plug-in" Interface Design
Users should be able to benchmark their code by following this pattern:

```cpp
// Example registration for Copilot to follow:
runner.Register("Swarm_Update", [](Params& p) {
    // 1. Setup (Untimed)
    World world;
    world.spawn(p.agent_count);

    // 2. Execution (Timed)
    BENCH_START();
    for(int i = 0; i < 100; ++i) {
        world.update_tick();
    }
    BENCH_STOP();

    // 3. Teardown (Untimed)
    world.cleanup();
});
```

## 4. Integration Strategy
* **Build System:** Independent Makefile target (`make benchmark`).
* **Flags:** Use `-O3` for benchmarks and `-D BENCHMARK_MODE` to disable GUI/SFML components.

---