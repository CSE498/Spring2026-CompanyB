/**
 * @file DataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Interfaces/IDataLog.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498 {

template <typename DataClass>
class DataLog : public IDataLog<DataClass> {
 private:
  std::unordered_map<std::string, std::vector<TickStats>> time_series;
  std::unordered_map<size_t, WorldPosition> previous_positions;
  std::unordered_map<size_t, double> cumulative_distance;
  std::unordered_map<size_t, size_t> active_ticks;
  std::unordered_set<size_t> completed_agents;

  static constexpr std::array<std::string_view, 4> kInfectionFields = {
      "infection_count", "susceptible_count", "cured_count",
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

  ~DataLog() = default;

  /// @brief Aggregate agent states for the current tick; appends one TickStats
  /// entry per declared field to the time series.
  /// @param agents Vector of agents in the world
  void AggregateData(
      const std::vector<std::shared_ptr<StepAgentBase<DataClass>>>& agents)
      override;

  /// @brief Returns the full time series for all declared fields.
  /// Index into the vector by tick number; call .back() for the latest tick.
  /// @returns Map of field name to vector of TickStats (one entry per tick)
  const std::unordered_map<std::string, std::vector<TickStats>>&
  GetAggregationData() const override;
};

}  // namespace cse498
