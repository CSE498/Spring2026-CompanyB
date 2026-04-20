/**
 * @file IDataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/StepAgentBase.hpp"

namespace cse498 {

struct TickStats {
  double min    = 0.0;
  double max    = 0.0;
  double mean   = 0.0;
  double median = 0.0;
  double sum    = 0.0;
  size_t count  = 0;
};

enum class WorldType { Infection, Traffic };

/**
 * @class IDataLog
 * @brief Interface for data aggregation and storage.
 */
template <typename DataClass>
class IDataLog {
 public:
  virtual ~IDataLog() = default;

  /// @brief Aggregate agent states; should be called every tick
  /// @param agents Vector of agents in the world
  virtual void AggregateData(const std::vector<std::shared_ptr<StepAgentBase<DataClass>>>& agents) = 0;

  /// @brief Returns the full time series of aggregation data for all declared
  /// fields. Each vector entry corresponds to one tick (index 0 = first tick).
  /// @returns Map of field name to vector of TickStats (one per tick)
  virtual const std::unordered_map<std::string, std::vector<TickStats>>&
  GetAggregationData() const = 0;
};

}  // namespace cse498
