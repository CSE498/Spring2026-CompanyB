
#include "DataLog.hpp"

#include <algorithm>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace cse498 {

template <typename DataClass>
void DataLog<DataClass>::AggregateData(const std::vector<std::shared_ptr<StepAgentBase<DataClass>>>& agents) {
  // Collect per-field samples from all agents for this tick
  std::unordered_map<std::string, std::vector<double>> samples;
  for (const auto& [fieldName, unused] : time_series) {
    samples[fieldName] = {};
  }

  for (const auto& agent : agents) {
    // TODO: replace with toJson() or describe() once agent teams finalize their API
    // auto state = agent.describe();
    // for (const auto& [fieldView, value] : state) {
    //   std::string fieldKey(fieldView);
    //   if (samples.contains(fieldKey)) {
    //     samples[fieldKey].push_back(value);
    //   }
    // }
  }

  // Compute TickStats for each declared field and append to the time series
  for (auto& [fieldName, values] : samples) {
    TickStats stats;

    if (values.empty()) {
      time_series[fieldName].push_back(stats);  // push zeroed stats
      continue;
    }

    stats.count = values.size();
    stats.sum   = std::accumulate(values.begin(), values.end(), 0.0);
    stats.mean  = stats.sum / static_cast<double>(stats.count);
    stats.min   = *std::min_element(values.begin(), values.end());
    stats.max   = *std::max_element(values.begin(), values.end());

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

template <typename DataClass>
const std::unordered_map<std::string, std::vector<TickStats>>&
DataLog<DataClass>::GetAggregationData() const {
  return time_series;
}

}  // namespace cse498
