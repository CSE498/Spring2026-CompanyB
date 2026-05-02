#include "BenchRunner.hpp"
#include "MetricCollector.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <system_error>
#include <vector>

namespace cse498::benchmarking {

extern void SetCurrentCollector(MetricCollector* collector);

using BenchmarkCallable = std::function<void(Params&)>;

namespace {

/**
 * @brief A helper struct to ensure that the thread-local current MetricCollector is reset to nullptr when going out of scope.
 */
struct CollectorScope {
  ~CollectorScope() {
    SetCurrentCollector(nullptr);
  }
};

/**
 * @brief A helper struct to hold aggregated metric results, including average, minimum, maximum, and standard deviation.
 */
struct AggregatedMetric {
  std::uint64_t average{0};
  std::uint64_t minimum{0};
  std::uint64_t maximum{0};
  std::uint64_t stdev{0};
};

/**
 * @brief Aggregates a specific metric from a set of samples, computing average, min, max, and standard deviation.
 * @param samples The vector of MetricSample instances to aggregate.
 * @param T A callable type that takes a MetricSample and returns the metric value to aggregate.
 * @return An AggregatedMetric instance containing the aggregated metrics for the specified metric.
 */
template <typename T>
AggregatedMetric AggregateMetric(const std::vector<MetricSample>& samples, T  accessor) {
  AggregatedMetric metric{};
  if (samples.empty()) {
    return metric;
  }

  const auto first_value = accessor(samples.front());
  metric.minimum = first_value;
  metric.maximum = first_value;

  long double sum = 0.0L;
  long double sum_squares = 0.0L;

  for (const auto& sample : samples) {
    const auto value = static_cast<long double>(accessor(sample));
    const auto value_uint = static_cast<std::uint64_t>(value);
    metric.minimum = std::min(metric.minimum, value_uint);
    metric.maximum = std::max(metric.maximum, value_uint);
    sum += value;
    sum_squares += value * value;
  }

  const auto count = static_cast<long double>(samples.size());
  const auto mean = sum / count;
  metric.average = static_cast<std::uint64_t>(std::llround(mean));

  long double variance = 0.0L;
  if (samples.size() > 1U) {
    variance = (sum_squares - (sum * sum) / count) / (count - 1.0L);
  }

  if (variance < 0.0L) {
    variance = 0.0L;
  }

  metric.stdev = static_cast<std::uint64_t>(std::llround(std::sqrt(variance)));
  return metric;
}

/**
 * @brief Aggregates a set of MetricSample instances into a BenchmarkResult, computing average, min, max, and standard deviation for both wall time and memory usage.
 * @param samples The vector of MetricSample instances to aggregate.
 * @param params The Params instance containing the parameters of the benchmark, used to populate the agent count in the result.
 * @return A BenchmarkResult instance containing the aggregated metrics.
 */
BenchmarkResult AggregateResult(const std::vector<MetricSample>& samples, const Params& params) {
  BenchmarkResult result{};
  result.agent_count = params.agent_count;
  result.sample_count = static_cast<std::uint32_t>(samples.size());

  const auto wall_time = AggregateMetric(samples, [](const MetricSample& sample) {
    return sample.wall_time_ns;
  });
  const auto memory_usage = AggregateMetric(samples, [](const MetricSample& sample) {
    return sample.memory_usage_kb;
  });
  const auto current_rss_at_stop = AggregateMetric(samples, [](const MetricSample& sample) {
    return sample.current_rss_at_stop_kb;
  });

  result.avg_wall_time_ns = wall_time.average;
  result.min_wall_time_ns = wall_time.minimum;
  result.max_wall_time_ns = wall_time.maximum;
  result.stdev_wall_time_ns = wall_time.stdev;
  result.avg_memory_usage_kb = memory_usage.average;
  result.min_memory_usage_kb = memory_usage.minimum;
  result.max_memory_usage_kb = memory_usage.maximum;
  result.stdev_memory_usage_kb = memory_usage.stdev;
  result.avg_current_rss_at_stop_kb = current_rss_at_stop.average;
  result.min_current_rss_at_stop_kb = current_rss_at_stop.minimum;
  result.max_current_rss_at_stop_kb = current_rss_at_stop.maximum;
  result.stdev_current_rss_at_stop_kb = current_rss_at_stop.stdev;
  return result;
}

}  // namespace

[[nodiscard]] RegistrationResult BenchRunner::Register(std::string benchmark_id, BenchmarkCallable callable) {
  if (benchmark_id.empty()) {
    return RegistrationResult::InvalidId;
  }

  if (!callable) {
    return RegistrationResult::InvalidCallable;
  }

  auto has_it = registry.find(benchmark_id);
  if (has_it != registry.end()) {
    has_it->second = std::move(callable);
    return RegistrationResult::Replaced;
  }

  registry.emplace(std::move(benchmark_id), std::move(callable));
  return RegistrationResult::Inserted;
}

[[nodiscard]] bool BenchRunner::HasBenchmark(std::string_view  benchmark_id) const {
  if (benchmark_id.empty()) {
    return false;
  }

  return registry.find(benchmark_id) != registry.end();
}

[[nodiscard]] std::size_t BenchRunner::Size() const noexcept {
  return registry.size();
}

[[nodiscard]] std::vector<std::string> BenchRunner::ListBenchmarkIds() const {
  std::vector<std::string> ids;
  for (const auto& [id, _] : registry) {
    ids.push_back(id);
  }
  return ids;
}

[[nodiscard]] RunStatus BenchRunner::RunBenchmarkById(std::string_view benchmark_id, const Params& params) {
  if (benchmark_id.empty()) {
    has_last_result = false;
    return RunStatus::UnknownBenchmark;
  }

  if (!params.IsValid()) {
    has_last_result = false;
    return RunStatus::InvalidParams;
  }

  auto bench_obj = registry.find(benchmark_id);
  if (bench_obj == registry.end()) {
    has_last_result = false;
    return RunStatus::UnknownBenchmark;
  }

  MetricCollector collector{};
  SetCurrentCollector(&collector);
  CollectorScope collector_scope{};
  has_last_result = false;

  try {
    // Warmup phase (untimed)
    for (std::uint32_t i = 0; i < params.warmup_ticks; ++i) {
      auto params_copy = params;
      bench_obj->second(params_copy);
    }
    
    // Measurement phase
    std::vector<MetricSample> samples;
    for (std::uint32_t i = 0; i < params.repetitions; ++i) {
      auto params_copy = params;
      bench_obj->second(params_copy);
      
      auto sample = GetLastSample();
      if (sample.success) {
        samples.push_back(sample);
      }
    }
    
    SetCurrentCollector(nullptr);
    
    if (samples.empty()) {
      return RunStatus::MetricFailure;
    }

    last_result = AggregateResult(samples, params);
    has_last_result = true;

  } catch (const std::exception& e) {
    return RunStatus::BenchmarkFailure;
  } catch (...) {
    return RunStatus::BenchmarkFailure;
  }

  return RunStatus::Ok;
}

[[nodiscard]] RunAndReportResult BenchRunner::RunBenchmarkAndWriteReport(
    std::string_view benchmark_id,
    const Params& params,
    std::string_view output_file_name,
    std::filesystem::path output_dir,
    ReportFormat format) {
  RunAndReportResult result{};
  result.run_status = RunStatus::BenchmarkFailure;
  result.report_status = ReportStatus::WriteFailed;

  if (output_file_name.empty()) {
    result.status = RunAndReportStatus::InvalidOutputFileName;
    return result;
  }

  std::filesystem::path file_name{output_file_name};
  if (file_name.has_parent_path() || file_name.is_absolute()) {
    result.status = RunAndReportStatus::InvalidOutputFileName;
    return result;
  }

  if (!file_name.has_extension()) {
    file_name += (format == ReportFormat::Csv) ? ".csv" : ".json";
  }

  result.output_path = output_dir / file_name;

  std::error_code ec;
  std::filesystem::create_directories(output_dir, ec);
  if (ec) {
    result.status = RunAndReportStatus::DirectoryCreateFailed;
    return result;
  }

  result.run_status = RunBenchmarkById(benchmark_id, params);
  if (result.run_status != RunStatus::Ok) {
    result.status = RunAndReportStatus::RunFailed;
    return result;
  }

  const auto benchmark_result = GetLastResult();
  if (!benchmark_result.has_value()) {
    result.status = RunAndReportStatus::MissingResult;
    return result;
  }

  const BenchmarkReportEntry entry{
      std::string{benchmark_id},
      params,
      *benchmark_result,
  };

  ReportGenerator generator{};
  result.report_status =
      generator.Write(result.output_path, format, std::array{entry});

  if (result.report_status != ReportStatus::Ok) {
    result.status = RunAndReportStatus::ReportWriteFailed;
    return result;
  }

  result.status = RunAndReportStatus::Ok;
  return result;
}

[[nodiscard]] std::optional<BenchmarkResult> BenchRunner::GetLastResult() const noexcept {
  if (!has_last_result) {
    return std::nullopt;
  }

  return last_result;
}


}