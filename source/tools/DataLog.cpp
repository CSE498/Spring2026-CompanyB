
#include "DataLog.hpp"
#include <vector>

namespace cse498 {

template <DataConcept DataClass>
void DataLog<DataClass>::AggregateData(const std::vector<AgentBase>& agents) {
  Clear();
  
  for (size_t i = 0; i < agents.size(); ++i) {
    // TODO: Extract agent state and call AddData
    // auto state = agents[i].describe();
    // AddData(state, i);
  }
}

template <DataConcept DataClass>
std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> DataLog<DataClass>::GetAggregationData() const {
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, double>> totalData = {};
    for (const auto& [fieldName, value] : min) {
        totalData[fieldName] = {};
        totalData[fieldName]["min"] = value;
    }
    for (const auto& [fieldName, value] : max) {
        totalData[fieldName]["max"] = value;
    }
    for (const auto& [fieldName, value] : mean) {
        totalData[fieldName]["mean"] = value;
    }
    for (const auto& [fieldName, value] : median) {
        totalData[fieldName]["median"] = value;
    }
    for (const auto& [fieldName, value] : sum) {
        totalData[fieldName]["sum"] = value;
    }
    for (const auto& [fieldName, value] : count_nonzero) {
        totalData[fieldName]["count_nonzero"] = value;
    }
    return totalData;
  }
  
  template <DataConcept DataClass>
  void DataLog<DataClass>::Clear() {
    min.clear();
    max.clear();
    mean.clear();
    median.clear();
    sum.clear();
  }
}

