#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "BenchmarkTypes.hpp"
#include "ReportGenerator.hpp"

namespace cse498::benchmarking {

using BenchmarkCallable = std::function<void(Params&)>;

enum class RunAndReportStatus {
  Ok,
  RunFailed,
  MissingResult,
  InvalidOutputFileName,
  DirectoryCreateFailed,
  ReportWriteFailed,
};

struct RunAndReportResult {
  RunAndReportStatus status{RunAndReportStatus::RunFailed};
  RunStatus run_status{RunStatus::BenchmarkFailure};
  ReportStatus report_status{ReportStatus::WriteFailed};
  std::filesystem::path output_path{};
};

class BenchRunner {
 private:
  std::map<std::string, BenchmarkCallable, std::less<>> registry{};
  BenchmarkResult last_result{};
  bool has_last_result{false};

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

  [[nodiscard]] RunAndReportResult RunBenchmarkAndWriteReport(
      std::string_view benchmark_id,
      const Params& base_params,
      std::string_view output_file_name,
      std::filesystem::path output_dir = "../benchmarking/groups/results",
      ReportFormat format = ReportFormat::Csv);

  [[nodiscard]] std::optional<BenchmarkResult> GetLastResult() const noexcept;
};

}  // namespace cse498::benchmarking
