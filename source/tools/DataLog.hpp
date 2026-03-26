/**
 * @file DataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../Interfaces/IDataLog.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {
/* Class Description:
 *  DataLog is the center of data aggregation and storage for the simulation.
 *  It collects, organizes, and analyzes real-time action data from agents.
 *  DataLog is a non-copyable (copy constructor and assignment operator
 *  are deleted) class intentionally to ensure it acts as a unique "global"
 aggregator
 *  and logging system. It's a singleton according to our current design
 pattern.

 */
class DataLog : public IDataLog {
 private:
  /// @brief Vector storing all log entries as JSON objects
  std::vector<nlohmann::json> mEntries{};

  /// @brief Running sum for efficient mean calculation
  double mRunningSum = 0.0;

  /// @brief Minimum recorded value from entry data
  double mMinValue = 0.0;

  /// @brief Maximum recorded value from entry data
  double mMaxValue = 0.0;

  /// @brief Total count of entries with "duration" field
  size_t mCount = 0;

  /// @brief Sorted container of durations for efficient median calculation
  /// (added after peer review). Future: Can be refactored to
  /// std::map<std::string, std::multiset<double>> mFieldValues to support
  /// median calculations for multiple fields (e.g., duration, infection
  /// thresholds etc).
  std::multiset<double> mDurations{};

  /// @brief Cached median value to avoid O(n) multiset traversal on frequent
  /// reads
  mutable double mMedianCache = 0.0;

  /// @brief Flag indicating whether median cache is valid (true = cache is
  /// up-to-date)
  mutable bool mMedianIsValid = false;

 public:
  /// @brief Constructor (default)
  DataLog() = default;

  /// @brief Destructor
  ~DataLog() = default;

  /// @brief Copy constructor (disabled)
  DataLog(const DataLog&) = delete;

  /// @brief Assignment operator (disabled)
  DataLog& operator=(const DataLog&) = delete;

  // / @brief Adds a log entry with automatic timestamp.
  // / @param data ActionEventBase containing agentId, actionType, duration, and
  // / summary.
  // void AddEntry(const ActionEventBase& data) override;

  /// @brief Gets the mean of all logged action durations.
  /// @return Mean duration value, or std::nullopt if no data has been logged.
  std::optional<double> GetMean() const override;

  /// @brief Gets the median of all logged action durations.
  /// @return Median duration value, or std::nullopt if no data has been logged.
  std::optional<double> GetMedian() const override;

  /// @brief Gets the minimum logged action duration.
  /// @return Minimum duration value, or std::nullopt if no data has been
  /// logged.
  std::optional<double> GetMin() const override;

  /// @brief Gets the maximum logged action duration.
  /// @return Maximum duration value, or std::nullopt if no data has been
  /// logged.
  std::optional<double> GetMax() const override;

  /// @brief Gets the total number of logged entries.
  /// @return Total entry count.
  size_t GetCount() const override;

  /// @brief Gets all logged entries.
  /// @return Const reference to the entries vector.
  const std::vector<nlohmann::json>& GetEntries() const;

  /// @brief Clears all entries and resets statistics.
  void Reset() override;
};

}  // namespace cse498