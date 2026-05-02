#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

#include "BenchmarkTypes.hpp"

namespace cse498::benchmarking {

enum class ReportFormat {
  Csv,
  Json,
};

enum class ReportStatus {
  Ok,
  EmptyResults,
  UnsupportedFormat,
  OpenFailed,
  WriteFailed,
};

struct BenchmarkReportEntry {
  std::string benchmark_name{};
  Params params{};
  BenchmarkResult result{};
};

class ReportGenerator {
 public:
  [[nodiscard]] ReportStatus Write(std::filesystem::path output_path,
                                   ReportFormat format,
                                   std::span<const BenchmarkReportEntry> entries) const;

  [[nodiscard]] ReportStatus WriteCsv(std::filesystem::path output_path,
                                      std::span<const BenchmarkReportEntry> entries) const;

  [[nodiscard]] ReportStatus WriteJson(std::filesystem::path output_path,
                                       std::span<const BenchmarkReportEntry> entries) const;
};

}  // namespace cse498::benchmarking