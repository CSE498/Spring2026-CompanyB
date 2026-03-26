/**
 * @file IDataLog.hpp
 * @author Divyalakshmi Varadha Rajan Prem Sudha
 **/

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "IActionLog.hpp"

namespace cse498 {

/**
 * @class IDataLog
 * @brief Interface for data aggregation and storage.
 */
class IDataLog {
 public:
  virtual ~IDataLog() = default;

  /**
   * @brief Adds a log entry.
   * @param data JSON object containing entry details.
   */
  // virtual void AddEntry(const ActionEventBase& data) = 0;

  /**
   * @brief Gets the mean of all logged action durations.
   * @return Mean duration value, or std::nullopt if no data has been logged.
   */
  virtual std::optional<double> GetMean() const = 0;

  /**
   * @brief Gets the median of all logged action durations.
   * @return Median duration value, or std::nullopt if no data has been logged.
   */
  virtual std::optional<double> GetMedian() const = 0;

  /**
   * @brief Gets the minimum logged action duration.
   * @return Minimum duration value, or std::nullopt if no data has been logged.
   */
  virtual std::optional<double> GetMin() const = 0;

  /**
   * @brief Gets the maximum logged action duration.
   * @return Maximum duration value, or std::nullopt if no data has been logged.
   */
  virtual std::optional<double> GetMax() const = 0;

  /**
   * @brief Gets the total number of logged entries.
   * @return Total entry count.
   */
  virtual size_t GetCount() const = 0;

  /**
   * @brief Clears all entries and resets statistics.
   */
  virtual void Reset() = 0;
};

}  // namespace cse498
