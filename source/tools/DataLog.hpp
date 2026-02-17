/**
 * This file serves as the center of data aggregation and storage
 * for the Data Analytics group.
 * @brief An internal class to collect, organize, analyze and store
 * real-time data.
 * @note Status: INITIAL DESIGN
 **/

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace cse498 {
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

  /// @brief Adds a log entry from a JSON object
  /// @param data JSON object with required fields:
  ///             agentId (string), actionType (string),
  ///             duration (double), summary (string)
  /// @note Timestamp is automatically added from GlobalClock::GetTime()
  void AddEntry(const nlohmann::json& data);
  /// @brief Returns the mean of all action durations
  /// @return Mean duration, or 0.0 if no entries exist
  double GetMean() const;

  /// @brief Returns the median of all action durations
  /// @return Median duration, or 0.0 if no entries exist
  double GetMedian() const;

  /// @brief Returns the minimum action duration
  /// @return Minimum duration, or 0.0 if no entries exist
  double GetMin() const;

  /// @brief Returns the maximum action duration
  /// @return Maximum duration, or 0.0 if no entries exist
  double GetMax() const;

  /// @brief Returns the total number of logged actions
  /// @return Count of entries
  size_t GetCount() const;

  /// @brief Gets all log entries
  /// @return Const reference to the entries vector
  const std::vector<nlohmann::json>& GetEntries() const;

  /// @brief Clears all stored data and resets internal statistics
  void Reset();
};

}  // namespace cse498
