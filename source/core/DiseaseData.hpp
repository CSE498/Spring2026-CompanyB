/**
 * DiseaseData.hpp
 * @brief Agent state DataClass for infectious-disease worlds.
 *
 * Satisfies the IsDataClass concept so that StepAgentBase<DiseaseData> and
 * StepWorldBase<DiseaseData> can be instantiated.
 **/

#pragma once

#include "WorldPosition.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

/// Health states for an agent in an infectious-disease simulation.
enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

/// All per-agent state for the infectious-world simulation.
/// Position, health status, and how long the agent has been in that state
/// are bundled here so they flow through StepWorldBase's DoAction / SetState
/// machinery without a separate health_map.
struct DiseaseData {
  WorldPosition position{};
  HealthState health{HealthState::SUSCEPTIBLE};
  size_t ticks_in_state{0};

  nlohmann::json ToJson() const {
    return nlohmann::json{
        {"position", position.ToJson()},
        {"health", health},
        {"ticks_in_state", ticks_in_state},
    };
  }

  DiseaseData(const nlohmann::json& j) {
    position = WorldPosition(j.at("position"));
    health = j.at("health").get<HealthState>();
    ticks_in_state = j.at("ticks_in_state").get<size_t>();
  }
  
};

}  // namespace cse498
