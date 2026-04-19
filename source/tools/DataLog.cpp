
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
    nlohmann::json state = agent->GetState().ToJSON();
    for (auto it = state.begin(); it != state.end(); ++it) {
      std::string fieldKey = it.key();
      if (samples.contains(fieldKey)) {
        if (it.value().is_number()) {
          samples[fieldKey].push_back(it.value().get<double>());
        } else if (it.value().is_boolean()) {
          samples[fieldKey].push_back(it.value().get<bool>() ? 1.0 : 0.0);
        }
      }
    }
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
