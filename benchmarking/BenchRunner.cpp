#include "BenchRunner.hpp"

namespace cse498::benchmarking {

 using BenchmarkCallable = std::function<void(Params&)>;

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
  
  if (registry.find(benchmark_id) != registry.end()) {
    return true;
  }
  
    return false;
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
    return RunStatus::UnknownBenchmark;
  }

  if (!params.IsValid()) {
    return RunStatus::InvalidParams;
  }

  auto bench_obj = registry.find(benchmark_id);
  if (bench_obj == registry.end()) {
    return RunStatus::UnknownBenchmark;
  }

  try {
    // Warmup phase
    for (std::uint32_t i = 0; i < params.warmup_ticks; ++i) {
      auto params_copy = params;
      bench_obj->second(params_copy);
    }
    
    // Measurement phase
    auto params_copy = params;
    bench_obj->second(params_copy);

  } catch (const std::exception& e) {
    return RunStatus::BenchmarkFailure;
  } catch (...) {
    return RunStatus::BenchmarkFailure;
  }

  return RunStatus::Ok;
}


}