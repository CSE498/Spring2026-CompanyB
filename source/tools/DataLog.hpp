/**
 * @file DataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace cse498 {
/* Class Description:
 *  DataLog is the center of data aggregation and storage for the simulation.
 *  It collects, organizes, and analyzes real-time action data from agents.
 */
class DataLog {
 private:
  /// @brief Vector storing all log entries as JSON objects
  std::vector<nlohmann::json> mEntries;

  /// @brief Running sum for efficient mean calculation
  double mRunningSum = 0.0;

  /// @brief Minimum recorded value from entry data
  double mMinValue = 0.0;

  /// @brief Maximum recorded value from entry data
  double mMaxValue = 0.0;

  /// @brief Total count of entries with "value" field
  size_t mCount = 0;

  /// @brief Flag to track if any value entries have been logged
  bool mHasData = false;

 public:
  /// @brief Constructor (default)
  DataLog() = default;

  /// @brief Destructor
  ~DataLog() = default;

  /// @brief Copy constructor (disabled)
  DataLog(const DataLog&) = delete;

  /// @brief Assignment operator (disabled)
  DataLog& operator=(const DataLog&) = delete;

  /// @brief Adds a log entry with automatic timestamp.
  /// @param data JSON object containing agentId, actionType, duration, and
  /// summary.
  void AddEntry(const nlohmann::json& data);
  /// @brief Gets the mean of all logged action durations.
  /// @return Mean duration value.
  double GetMean() const;

  /// @brief Gets the median of all logged action durations.
  /// @return Median duration value.
  double GetMedian() const;

  /// @brief Gets the minimum logged action duration.
  /// @return Minimum duration value.
  double GetMin() const;

  /// @brief Gets the maximum logged action duration.
  /// @return Maximum duration value.
  double GetMax() const;

  /// @brief Gets the total number of logged entries.
  /// @return Total entry count.
  size_t GetCount() const;

  /// @brief Gets all logged entries.
  /// @return Const reference to the entries vector.
  const std::vector<nlohmann::json>& GetEntries() const;

  /// @brief Clears all entries and resets statistics.
  void Reset();
};

}  // namespace cse498