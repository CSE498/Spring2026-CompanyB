#include "ReportGenerator.hpp"

#include <cstddef>
#include <fstream>
#include <sstream>
#include <utility>

namespace cse498::benchmarking {

namespace {

std::string EscapeCsvField(std::string_view value) {
  std::string escaped;
  escaped.reserve(value.size() + 2U);
  escaped.push_back('"');
  for (const char character : value) {
    if (character == '"') {
      escaped.push_back('"');
    }
    escaped.push_back(character);
  }
  escaped.push_back('"');
  return escaped;
}

std::string EscapeJsonString(std::string_view value) {
  std::string escaped;
  escaped.reserve(value.size() + 8U);
  for (const char character : value) {
    switch (character) {
      case '"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\b': escaped += "\\b"; break;
      case '\f': escaped += "\\f"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        escaped.push_back(character);
        break;
    }
  }
  return escaped;
}

bool WriteLine(std::ofstream& output, std::string_view line) {
  output << line << '\n';
  return static_cast<bool>(output);
}

void WriteJsonParams(std::ofstream& output, const Params& params) {
  output << "\"params\":{";
  output << "\"benchmark_name\":\"" << EscapeJsonString(params.benchmark_name) << "\",";
  output << "\"agent_count\":" << params.agent_count << ',';
  output << "\"tick_count\":" << params.tick_count << ',';
  output << "\"warmup_ticks\":" << params.warmup_ticks << ',';
  output << "\"repetitions\":" << params.repetitions << ',';
  output << "\"seed\":" << params.seed;
  output << "}";
}

void WriteJsonResult(std::ofstream& output, const BenchmarkResult& result) {
  output << "\"result\":{";
  output << "\"agent_count\":" << result.agent_count << ',';
  output << "\"avg_wall_time_ns\":" << result.avg_wall_time_ns << ',';
  output << "\"min_wall_time_ns\":" << result.min_wall_time_ns << ',';
  output << "\"max_wall_time_ns\":" << result.max_wall_time_ns << ',';
  output << "\"stdev_wall_time_ns\":" << result.stdev_wall_time_ns << ',';
  output << "\"avg_memory_usage_kb\":" << result.avg_memory_usage_kb << ',';
  output << "\"min_memory_usage_kb\":" << result.min_memory_usage_kb << ',';
  output << "\"max_memory_usage_kb\":" << result.max_memory_usage_kb << ',';
  output << "\"stdev_memory_usage_kb\":" << result.stdev_memory_usage_kb << ',';
  output << "\"sample_count\":" << result.sample_count;
  output << "}";
}

}  // namespace

ReportStatus ReportGenerator::Write(std::filesystem::path output_path,
                                    ReportFormat format,
                                    std::span<const BenchmarkReportEntry> entries) const {
  switch (format) {
    case ReportFormat::Csv:
      return WriteCsv(std::move(output_path), entries);
    case ReportFormat::Json:
      return WriteJson(std::move(output_path), entries);
    default:
      return ReportStatus::UnsupportedFormat;
  }
}

ReportStatus ReportGenerator::WriteCsv(std::filesystem::path output_path,
                                       std::span<const BenchmarkReportEntry> entries) const {
  if (entries.empty()) {
    return ReportStatus::EmptyResults;
  }

  std::ofstream output(output_path, std::ios::out | std::ios::trunc);
  if (!output.is_open()) {
    return ReportStatus::OpenFailed;
  }

  if (!WriteLine(output,
                 "benchmark_name,agent_count,tick_count,warmup_ticks,repetitions,seed,avg_wall_time_ns,min_wall_time_ns,max_wall_time_ns,stdev_wall_time_ns,avg_memory_usage_kb,min_memory_usage_kb,max_memory_usage_kb,stdev_memory_usage_kb,sample_count")) {
    return ReportStatus::WriteFailed;
  }

  for (const auto& entry : entries) {
    std::ostringstream line;
    line << EscapeCsvField(entry.benchmark_name) << ','
         << entry.params.agent_count << ','
         << entry.params.tick_count << ','
         << entry.params.warmup_ticks << ','
         << entry.params.repetitions << ','
         << entry.params.seed << ','
         << entry.result.avg_wall_time_ns << ','
         << entry.result.min_wall_time_ns << ','
         << entry.result.max_wall_time_ns << ','
         << entry.result.stdev_wall_time_ns << ','
         << entry.result.avg_memory_usage_kb << ','
         << entry.result.min_memory_usage_kb << ','
         << entry.result.max_memory_usage_kb << ','
         << entry.result.stdev_memory_usage_kb << ','
         << entry.result.sample_count;

    if (!WriteLine(output, line.str())) {
      return ReportStatus::WriteFailed;
    }
  }

  output.flush();
  if (!output.good()) {
    return ReportStatus::WriteFailed;
  }

  return ReportStatus::Ok;
}

ReportStatus ReportGenerator::WriteJson(std::filesystem::path output_path,
                                        std::span<const BenchmarkReportEntry> entries) const {
  if (entries.empty()) {
    return ReportStatus::EmptyResults;
  }

  std::ofstream output(output_path, std::ios::out | std::ios::trunc);
  if (!output.is_open()) {
    return ReportStatus::OpenFailed;
  }

  output << "{\"benchmarks\":[";
  for (std::size_t index = 0; index < entries.size(); ++index) {
    const auto& entry = entries[index];
    output << '{';
    output << "\"benchmark_name\":\"" << EscapeJsonString(entry.benchmark_name) << "\",";
    WriteJsonParams(output, entry.params);
    output << ',';
    WriteJsonResult(output, entry.result);
    output << '}';
    if (index + 1U < entries.size()) {
      output << ',';
    }
  }
  output << "]}";

  output.flush();
  if (!output.good()) {
    return ReportStatus::WriteFailed;
  }

  return ReportStatus::Ok;
}

}  // namespace cse498::benchmarking