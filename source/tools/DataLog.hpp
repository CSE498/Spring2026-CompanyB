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
#include <unordered_map>
#include <algorithm>

#include "../Interfaces/IDataLog.hpp"

namespace cse498 {
template <DataConcept DataClass>
class DataLog : public IDataLog<DataClass> {
 private:
  std::unordered_map<std::string_view, float> min = {};
  std::unordered_map<std::string_view, float> max = {};
  std::unordered_map<std::string_view, float> mean = {};
  std::unordered_map<std::string_view, float> median = {};
  std::unordered_map<std::string_view, float> sum = {};
  std::unordered_map<std::string_view, float> count_nonzero = {};
  std::unordered_map<std::string_view, std::vector<double>> median_samples = {};

  void Clear();

  std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> reshapeData() const;

  /// @brief
  /// @param agentState
  /// @param num_agents_counted Number of states counted, not including this state i.e. start at 0
  void AddData(std::unordered_map<std::string_view, double>& agentState, size_t num_agents_counted) {
    for (const auto& [fieldName, value] : agentState) {
        // Min
        if (min.contains(fieldName)) {
            min[fieldName] = std::min(min[fieldName], static_cast<float>(value));
        }
        else {
            min[fieldName] = static_cast<float>(value);
        }
        // Max
        if (max.contains(fieldName)) {
            max[fieldName] = std::max(max[fieldName], static_cast<float>(value));
        }
        else {
            max[fieldName] = static_cast<float>(value);
        }
        // Mean
        if (mean.contains(fieldName)) {
            mean[fieldName] = (mean[fieldName] * num_agents_counted + value) / (num_agents_counted + 1);
        }
        else {
            mean[fieldName] = value;
        }
        // Median
        if (median_samples.contains(fieldName)) {
            auto& samples = median_samples[fieldName];
            samples.push_back(value);
            std::vector<double> sortedSamples = samples;
            std::sort(sortedSamples.begin(), sortedSamples.end());
            const size_t n = sortedSamples.size();
            if (n % 2 == 0) {
                median[fieldName] = static_cast<float>(
                    (sortedSamples[n / 2 - 1] + sortedSamples[n / 2]) / 2.0);
            } else {
                median[fieldName] = static_cast<float>(sortedSamples[n / 2]);
            }
        }
        else {
            median_samples[fieldName].push_back(value);
            median[fieldName] = value;
        }
        // Sum
        if (mean.contains(fieldName)) {
            sum[fieldName] += value;
        }
        else {
            sum[fieldName] = value;
        }
        // Count of Non Zero Fields
        if (mean.contains(fieldName)) {
            count_nonzero[fieldName] += static_cast<double>(value != 0);
        }
        else {
            count_nonzero[fieldName] = static_cast<double>(value != 0);
        }
    }
  }
    
 public:
  DataLog();
  ~DataLog() = default;

  /// @brief Aggregate agent states; should be called every tick
  /// @param agents Vector of agents in the world
  void AggregateData(const std::vector<AgentBase>& agents);

  /// @brief Returns all aggregation data for all fields for the most recent tick
  /// @returns Map of field name to map of aggregation type to aggregate value
  std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> GetAggregationData() const override;
};

}  // namespace cse498