#include "MetricCollector.hpp"

#include <cstdio>
#include <optional>
#include <unistd.h>
#include <thread>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

namespace cse498::benchmarking {

// Helper to read current RSS in KB.
static std::optional<std::uint64_t> ReadRSSKB() {
#if defined(__linux__)
  // /proc/self/statm format: size resident shared text lib data dt
  std::FILE* file = std::fopen("/proc/self/statm", "r");
  if (file == nullptr) {
    return std::nullopt;
  }

  unsigned long pages_total = 0;
  unsigned long pages_resident = 0;
  const int scanned = std::fscanf(file, "%lu %lu", &pages_total, &pages_resident);
  std::fclose(file);

  if (scanned != 2) {
    return std::nullopt;
  }

  const long page_size = sysconf(_SC_PAGESIZE);
  if (page_size <= 0) {
    return std::nullopt;
  }

  const std::uint64_t bytes = static_cast<std::uint64_t>(pages_resident) *
                              static_cast<std::uint64_t>(page_size);
  return bytes / 1024ULL;
#elif defined(__APPLE__)
  // macOS fallback: ru_maxrss (best available without extra APIs).
  struct rusage usage {};
  if (getrusage(RUSAGE_SELF, &usage) != 0) {
    return std::nullopt;
  }
  return static_cast<std::uint64_t>(usage.ru_maxrss / 1024);
#else
  // Placeholder for non-Linux systems
  return std::nullopt;
#endif
}

MetricCollectorError MetricCollector::BENCH_START() {
  start_time = std::chrono::high_resolution_clock::now();
  has_start_time = true;
  has_stop_time = false;
  start_rss_captured = false;
  stop_rss_captured = false;
  const auto start_rss = ReadRSSKB();
  if (!start_rss.has_value()) {
    is_running = false;
    return MetricCollectorError::MemoryReadFailure;
  }

  start_rss_kb = *start_rss;
  start_rss_captured = true;
  is_running = true;
  return MetricCollectorError::Ok;
}

MetricCollectorError MetricCollector::BENCH_STOP() {
  if (!is_running) {
    return MetricCollectorError::TimingNotStarted;
  }
  stop_time = std::chrono::high_resolution_clock::now();
  has_stop_time = true;
  const auto stop_rss = ReadRSSKB();
  if (!stop_rss.has_value()) {
    stop_rss_captured = false;
    is_running = false;
    return MetricCollectorError::MemoryReadFailure;
  }

  stop_rss_kb = *stop_rss;
  stop_rss_captured = true;
  is_running = false;
  return MetricCollectorError::Ok;
}

MetricSample MetricCollector::GetSample() {
  MetricSample sample;
  
  if (!has_start_time) {
    sample.success = false;
    sample.error_message = "Timing was not properly started and stopped.";
    return sample;
  }

  if (!start_rss_captured) {
    sample.success = false;
    sample.error_message = "RSS capture failed.";
    return sample;
  }

  if (!has_stop_time || is_running || !stop_rss_captured || start_time >= stop_time) {
    sample.success = false;
    sample.error_message = "Timing was not properly started and stopped.";
    return sample;
  }

  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_time - start_time);
  sample.wall_time_ns = static_cast<std::uint64_t>(duration.count());
  
  // Compute current RSS delta (can be negative)
  std::int64_t rss_delta = static_cast<std::int64_t>(stop_rss_kb) - static_cast<std::int64_t>(start_rss_kb);
  sample.memory_usage_kb = static_cast<std::uint64_t>(rss_delta >= 0 ? rss_delta : 0);
  sample.current_rss_at_stop_kb = stop_rss_kb;
  
  sample.success = true;
  return sample;
}

// Thread-local access to the active MetricCollector
thread_local MetricCollector* g_current_collector = nullptr;

void SetCurrentCollector(MetricCollector* collector) {
  g_current_collector = collector;
}

MetricCollectorError BENCH_START() {
  if (!g_current_collector) {
    return MetricCollectorError::TimingNotStarted;
  }
  return g_current_collector->BENCH_START();
}

MetricCollectorError BENCH_STOP() {
  if (!g_current_collector) {
    return MetricCollectorError::TimingNotStarted;
  }
  return g_current_collector->BENCH_STOP();
}

MetricSample GetLastSample() {
  MetricSample sample;
  if (!g_current_collector) {
    sample.success = false;
    sample.error_message = "No active metric collector.";
    return sample;
  }
  return g_current_collector->GetSample();
}

}