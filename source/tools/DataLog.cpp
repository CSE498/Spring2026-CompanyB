
#include "DataLog.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "../core/AgentData.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

template <typename DataClass>
void DataLog<DataClass>::AggregateData(
    const std::vector<std::shared_ptr<StepAgentBase<DataClass>>>& agents) {
  // Collect per-field samples from all agents for this tick
  std::unordered_map<std::string, std::vector<double>> samples;
  for (const auto& [fieldName, unused] : time_series) {
    samples[fieldName] = {};
  }

  for (const auto& agent : agents) {
    nlohmann::json state = nlohmann::json::object();

    if constexpr (std::is_same_v<DataClass, DiseaseData>) {
      auto st = agent->GetState();
      const bool infected = st.health == HealthState::INFECTED;
      const bool susceptible = st.health == HealthState::SUSCEPTIBLE;
      const bool recovered = st.health == HealthState::RECOVERED;

      state = {
          {"infection_count", infected ? 1.0 : 0.0},
          {"susceptible_count", susceptible ? 1.0 : 0.0},
          {"cured_count", recovered ? 1.0 : 0.0},
          {"infection_probability", infected ? 1.0 : 0.0},
      };
    } else if constexpr (std::is_same_v<DataClass, TrafficData>) {
      auto st = agent->GetState();
      double eta = 0.0;
      const bool driving = st.is_active && st.destination.has_value();
      if (st.destination.has_value()) {
        const auto& d = st.destination.value();
        eta = std::abs(static_cast<double>(d.X() - st.position.X())) +
              std::abs(static_cast<double>(d.Y() - st.position.Y()));
      }

      state = {
          {"waiting_count", st.is_active ? 0.0 : 1.0},
          {"driving_count", driving ? 1.0 : 0.0},
          {"active_count", st.is_active ? 1.0 : 0.0},
          {"distance_driven", 0.0},
          {"time_to_arrive", eta},
      };
    }

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
    stats.sum = std::accumulate(values.begin(), values.end(), 0.0);
    stats.mean = stats.sum / static_cast<double>(stats.count);
    stats.min = *std::min_element(values.begin(), values.end());
    stats.max = *std::max_element(values.begin(), values.end());

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

// Explicit template instantiations for DiseaseData and TrafficData
template class DataLog<DiseaseData>;
template class DataLog<TrafficData>;

}  // namespace cse498
