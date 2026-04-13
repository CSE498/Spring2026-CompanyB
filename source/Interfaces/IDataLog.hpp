/**
 * @file IDataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498 {

template <typename DataClass>
concept DataConcept = requires(DataClass a) {
  {
    a.describe()
  } -> std::same_as<std::unordered_map<std::string_view, double>>;
};

/**
 * @class IDataLog
 * @brief Interface for data aggregation and storage.
 */
template <DataConcept DataClass>
class IDataLog {
 public:
  virtual ~IDataLog() = default;

  /// @brief Aggregate agent states; should be called every tick
  /// @param agents Vector of agents in the world
  virtual void AggregateData(const std::vector<AgentBase>& agents) = 0;

  /// @brief Returns all aggregation data for all fields for the most recent
  /// tick
  /// @returns Map of field name to map of aggregation type to aggregate value
  virtual std::unordered_map<std::string_view,
                             std::unordered_map<std::string_view, double>>
  GetAggregationData() const = 0;
};

}  // namespace cse498
