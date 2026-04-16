#pragma once

#include "Step.hpp"
#include "WorldPosition.hpp"
#include "core.hpp"
#include "../tools/Logger.hpp"

namespace cse498 {
using Concepts::IsDataClass;
using cse498::steps::StepContainer;

template <IsDataClass DataClass>
class StepAgentBase {
private:
  /// Unique id of the agent
  const size_t mId;

  ///Helper functions:
  WorldPosition DeserializePosition(const nlohmann::json& pos) {
    if (pos.contains("x") && pos.at("x").is_number_integer() &&
        pos.contains("y") && pos.at("y").is_number_integer()) {
      return WorldPosition{pos.at("x").get<int>(), pos.at("y").get<int>()};
    }
    return WorldPosition{0, 0};  // Default position if deserialization fails
  }

  Direction DeserializeDirection(const nlohmann::json& dir) {
    std::string direction = dir.get<std::string>();
    if (direction == "up") return Direction::Up;
    if (direction == "down") return Direction::Down;
    if (direction == "left") return Direction::Left;
    if (direction == "right") return Direction::Right;
    return Direction::Up;  // Default direction if deserialization fails
  }

  HealthState DeserializeHealthState(const nlohmann::json& state) {
    std::string healthState = state.get<std::string>();
    if (healthState == "susceptible") return HealthState::SUSCEPTIBLE;
    if (healthState == "infected") return HealthState::INFECTED;
    if (healthState == "recovered") return HealthState::RECOVERED;
    return HealthState::SUSCEPTIBLE;  // Default health state if deserialization fails
  }

  DataClass DeserializeTrafficData(const nlohmann::json& details) {
    DataClass data{};

    if (details.contains("destination") && details.at("destination").is_object()) {
      data.destination = DeserializePosition(details.at("destination"));
    }
    if (details.contains("position") && details.at("position").is_object()) {
      data.position = DeserializePosition(details.at("position"));
    }
    if (details.contains("direction") && details.at("direction").is_string()) {
      data.direction = DeserializeDirection(details.at("direction"));
    }
    if (details.contains("is_active") && details.at("is_active").is_boolean()) {
      data.is_active = details.at("is_active").get<bool>();
    }
    if (details.contains("symbol") && details.at("symbol").is_string()) {
      data.symbol = details.at("symbol").get<std::string>();
    }
    if (details.contains("colour") && details.at("colour").is_string()) {
      data.colour = details.at("colour").get<std::string>();
    }

    // Implementation for deserializing traffic data
    return data;
  }

  DataClass DeserializeDiseaseData(const nlohmann::json& details) {
    DataClass data{};
    
    if (details.contains("infection_probability") &&
        details.at("infection_probability").is_number()) {
      data.infection_probability = details.at("infection_probability").get<double>();
    }
    if (details.contains("infection_state") && details.at("infection_state").is_string()) {
      data.infection_state = DeserializeHealthState(details.at("infection_state"));
    }
    if (details.contains("destination") && details.at("destination").is_object()) {
      data.destination = DeserializePosition(details.at("destination"));
    }
    if (details.contains("position") && details.at("position").is_object()) {
      data.position = DeserializePosition(details.at("position"));
    }

    return data;
  }

 protected:
  /// Data class that holds agent data
  DataClass mData;

  /// Log of all actions taken by the agent, used for replay
  std::vector<ActionEvent<DataClass>> mActions;

  /// Cached string representation of agent ID to keep string_view (needed in logging) backing alive
  std::string mCachedAgentIdStr;

 public:
  StepAgentBase(DataClass data, size_t id)
      : mData{data}, mId{id}, mCachedAgentIdStr{std::to_string(id)} {}
  virtual ~StepAgentBase() = default;

  /// Get the ID of the agent
  [[nodiscard]] size_t getId() const noexcept { return mId; }

  [[nodiscard]] const std::vector<ActionEvent<DataClass>>& GetStates() const {
    return mActions;
  }

  void loadFromJson(const nlohmann::json& eventData) {
    if (!eventData.contains("actionType") ||
        !eventData.at("actionType").is_string()) {
      return;
    }
    if (eventData.at("actionType").get<std::string>() != "movement") {
      return;
    }
    if (!eventData.contains("timestamp") ||
        !eventData.at("timestamp").is_number_unsigned()) {
      return;
    }
    if (!eventData.contains("logLevel") ||
        !eventData.at("logLevel").is_number_integer()) {
      return;
    }

    const int levelRaw = eventData.at("logLevel").get<int>();
    if (levelRaw < static_cast<int>(LogLevel::Normal) ||
        levelRaw > static_cast<int>(LogLevel::Silent)) {
      return;
    }

    if (!eventData.contains("details") || !eventData.at("details").is_object()) {
      return;
    }

    DataClass details{};

    if constexpr (std::is_same_v<DataClass, TrafficData>) {
      details = this->DeserializeTrafficData(eventData.at("details"));
    } else if constexpr (std::is_same_v<DataClass, DiseaseData>) {
      details = this->DeserializeDiseaseData(eventData.at("details"));
    } 
    
    LogLevel logLevel = static_cast<LogLevel>(levelRaw);
    uint64_t tick = eventData.at("timestamp").get<uint64_t>();
    this->SetState(details, logLevel, tick);
  }

  // The main logic that separates the agents. When prompted for their turn,
  // this where the descisions are made
  [[nodiscard]] virtual StepContainer GetTurn() = 0;

  /*
   * Returning a copy is (most likely) correct here since DataClass is most
   * likely cheap to copy and we dont to modify the DataClass without passing
   * through SetState() since thats what logs the state changes for replay (?)
   */
  [[nodiscard]] DataClass GetState() const noexcept { return mData; }

  // Think: Should this also take in the log level and tick for replay purposes?
  // In regular usage, the world will need to pass the tick and log level for logging purposes.
  void SetState(DataClass data, LogLevel logLevel = LogLevel::Normal, uint64_t tick) {
    mData = data;
    // Here handle logic to log for replay?
    mActions.push_back(ActionEvent<DataClass>{.agentId = mCachedAgentIdStr,
                                              .actionType = "movement",
                                              .logLevel = logLevel,
                                              .timestamp = tick,
                                              .details = data});
  }

  virtual void SetGoal(WorldPosition position) = 0;
};

};  // namespace cse498
