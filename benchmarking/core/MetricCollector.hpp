#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#ifdef __linux__
#include <sys/resource.h>
#endif

#include "BenchmarkTypes.hpp"

namespace cse498::benchmarking {

class MetricCollector {
  private:
    std::chrono::high_resolution_clock::time_point start_time{};
    std::chrono::high_resolution_clock::time_point stop_time{};
    bool has_start_time{false};
    bool has_stop_time{false};
    std::uint64_t start_rss_kb{0};
    std::uint64_t stop_rss_kb{0};
    bool start_rss_captured{false};
    bool stop_rss_captured{false};
    bool is_running{false};

  public:
    MetricCollector() = default;
    ~MetricCollector() = default;

    MetricCollectorError BENCH_START();
    MetricCollectorError BENCH_STOP();
    MetricSample GetSample();
};

// Thread-local access to the active MetricCollector
MetricCollectorError BENCH_START();
MetricCollectorError BENCH_STOP();
MetricSample GetLastSample();

}