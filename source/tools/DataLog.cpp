
#include "DataLog.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "../core/AgentData.hpp"

namespace cse498 {

namespace {

void AddSample(std::unordered_map<std::string, std::vector<double>> &samples,
               const std::string &field, double value) {
  if (auto it = samples.find(field); it != samples.end()) {
    it->second.push_back(value);
  }
}

void AddSamples(std::unordered_map<std::string, std::vector<double>> &samples,
                const std::string &field, std::vector<double> values) {
  if (auto it = samples.find(field); it != samples.end()) {
    it->second = std::move(values);
  }
}

double Distance(const WorldPosition &a, const WorldPosition &b) {
  return std::hypot(a.X() - b.X(), a.Y() - b.Y());
}

} // namespace

template <typename DataClass>
void DataLog<DataClass>::AggregateData(
    const std::vector<std::shared_ptr<StepAgentBase<DataClass>>> &agents) {
  // Collect per-field samples from all agents for this tick
  std::unordered_map<std::string, std::vector<double>> samples;
  for (const auto &[fieldName, unused] : time_series) {
    samples[fieldName] = {};
  }

  if constexpr (std::is_same_v<DataClass, TrafficData>) {
    double active_count = 0.0;
    double driving_count = 0.0;
    double waiting_count = 0.0;
    std::vector<double> distance_samples;
    std::vector<double> arrival_samples;
    std::unordered_set<size_t> seen_agent_ids;

    for (const auto &agent : agents) {
      const size_t id = agent->GetId();
      const TrafficData state = agent->GetState();
      seen_agent_ids.insert(id);

      const auto previous_it = previous_positions.find(id);
      const bool has_previous = previous_it != previous_positions.end();
      const bool moved = has_previous && previous_it->second != state.position;

      if (has_previous && moved) {
        cumulative_distance[id] +=
            Distance(previous_it->second, state.position);
      } else {
        cumulative_distance.try_emplace(id, 0.0);
      }

      if (state.is_active) {
        ++active_count;
        ++active_ticks[id];
        if (moved) {
          ++driving_count;
        } else if (has_previous) {
          ++waiting_count;
        }
      } else if (has_previous && state.destination.has_value() &&
                 state.position == *state.destination &&
                 !completed_agents.contains(id)) {
        arrival_samples.push_back(
            static_cast<double>(active_ticks.try_emplace(id, 0).first->second));
        completed_agents.insert(id);
      }

      distance_samples.push_back(cumulative_distance[id]);
      previous_positions[id] = state.position;
    }

    AddSample(samples, "active_count", active_count);
    AddSample(samples, "driving_count", driving_count);
    AddSample(samples, "waiting_count", waiting_count);
    AddSamples(samples, "distance_driven", std::move(distance_samples));
    AddSamples(samples, "time_to_arrive", std::move(arrival_samples));

    for (auto it = previous_positions.begin();
         it != previous_positions.end();) {
      if (seen_agent_ids.contains(it->first)) {
        ++it;
      } else {
        cumulative_distance.erase(it->first);
        active_ticks.erase(it->first);
        completed_agents.erase(it->first);
        it = previous_positions.erase(it);
      }
    }
  }

  // Compute TickStats for each declared field and append to the time series
  for (auto &[fieldName, values] : samples) {
    TickStats stats;

    if (values.empty()) {
      time_series[fieldName].push_back(stats); // push zeroed stats
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
const std::unordered_map<std::string, std::vector<TickStats>> &
DataLog<DataClass>::GetAggregationData() const {
  return time_series;
}

} // namespace cse498

template class cse498::DataLog<cse498::TrafficData>;
template class cse498::DataLog<cse498::DiseaseData>;
