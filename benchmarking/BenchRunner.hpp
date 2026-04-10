#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "BenchmarkTypes.hpp"

namespace cse498::benchmarking {

using BenchmarkCallable = std::function<void(Params&)>;

class BenchRunner {
 private:
  std::map<std::string, BenchmarkCallable, std::less<>> registry{};

 public:
 BenchRunner() = default; 

 ~BenchRunner() = default;

 [[nodiscard]] RegistrationResult Register(std::string benchmark_id,
                                            BenchmarkCallable callable);

 [[nodiscard]] bool HasBenchmark(std::string_view benchmark_id) const;

 [[nodiscard]] std::size_t Size() const noexcept;

 [[nodiscard]] std::vector<std::string> ListBenchmarkIds() const;

  [[nodiscard]] RunStatus RunBenchmarkById(std::string_view benchmark_id,
                                           const Params& base_params);

  [[nodiscard]] RunStatus Banchmark();
};

}  // namespace cse498::benchmarking
