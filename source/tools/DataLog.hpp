/**
 * @file DataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../Interfaces/IDataLog.hpp"
#include "Worlds/WebTrafficWorld.hpp"

namespace cse498 {

template <typename World>
class DataLog : public IDataLog<World> {
 private:
  std::unordered_map<std::string, std::vector<TickStats>> time_series;

  static constexpr std::array<std::string_view, 4> kInfectionFields = {
      "infection_count", "susceptible_count", "recovered_count",
      "infection_probability"};

  static constexpr std::array<std::string_view, 5> kTrafficFields = {
      "waiting_count", "driving_count", "active_count", "distance_driven",
      "time_to_arrive"};

  void InitFields(std::span<const std::string_view> fields) {
    for (const auto& field : fields) {
      time_series[std::string(field)] = {};
    }
  }

 public:
  /// @brief Construct DataLog with a declared set of field names.
  /// The map is pre-keyed so the schema is known from tick 0.
  /// Only fields in this list will be aggregated; anything else from
  /// agent.describe() is ignored.
  /// @param fields constexpr array of field name string literals
  template <size_t N>
  explicit DataLog(const std::array<std::string_view, N>& fields) {
    InitFields(fields);
  }

  /// @brief Construct DataLog for a known world type.
  /// Intended for use by the web (emscripten) layer — selects the field list
  /// automatically based on the world.
  /// @param worldType WorldType::Infection or WorldType::Traffic
  explicit DataLog(WorldType worldType) {
    switch (worldType) {
      case WorldType::Infection:
        InitFields(kInfectionFields);
        break;
      case WorldType::Traffic:
        InitFields(kTrafficFields);
        break;
    }
  }

  ~DataLog() override = default;

  /// @brief Aggregate agent states for the current tick; appends one TickStats
  /// entry per declared field to the time series.
  /// @param world Reference to the world to aggregate data from
  void AggregateData(const World& world) override {
    // Collect per-field samples from the world for this tick
    std::unordered_map<std::string, std::vector<double>> samples;
    for (const auto& [fieldName, unused] : time_series) {
      samples[fieldName] = {};
    }

    if constexpr (std::is_same_v<World, WebTrafficWorld>) {
      samples["driving_count"].push_back(world.GetNumSpawnedAgents());
    }

    // Compute TickStats for each declared field and append to the time series
    for (auto& [fieldName, values] : samples) {
      TickStats stats;

      if (values.empty()) {
        time_series[fieldName].push_back(stats);  // push zeroed stats
        continue;
      }

      stats.current = values.back();
      stats.count = values.size();
      stats.sum = std::accumulate(values.begin(), values.end(), 0.0);
      stats.mean = stats.sum / static_cast<double>(stats.count);
      stats.min = *std::min_element(values.begin(), values.end());
      stats.max = *std::max_element(values.begin(), values.end());

      // Median
      std::vector<double> sorted = values;
      std::sort(sorted.begin(), sorted.end());
      const size_t n = sorted.size();
      if (n % 2 == 0) {
        stats.median = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
      } else {
        stats.median = sorted[n / 2];
      }

      time_series[fieldName].push_back(stats);
    }
  }

  /// @brief Returns the full time series for all declared fields.
  /// Index into the vector by tick number; call .back() for the latest tick.
  /// @returns Map of field name to vector of TickStats (one entry per tick)
  const std::unordered_map<std::string, std::vector<TickStats>>&
  GetAggregationData() const override {
    return time_series;
  }
};

}  // namespace cse498
