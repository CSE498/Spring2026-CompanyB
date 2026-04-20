#pragma once

#include <string>

#include "../Interfaces/IActionLog.hpp"
#include "Step.hpp"
#include "WorldPosition.hpp"
#include "core.hpp"

namespace cse498 {
using Concepts::IsDataClass;
using cse498::steps::StepContainer;

template <IsDataClass DataClass>
class StepAgentBase {
 private:
  /// Unique id of the agent
  const size_t mId;

 protected:
  /// Data class that holds agent data
  DataClass mData;

  /// Log of all states taken by the agent in JSON format, used for replay
  std::vector<nlohmann::json> mStates;

  /// Cached string representation of agent ID to keep string_view (needed in
  /// logging) backing alive
  std::string mCachedAgentIdStr;

  /// Flag indicating if the agent is currently replaying past states
  bool mIsReplay = false;

 public:
  StepAgentBase(DataClass data, size_t id, LogLevel logLevel = LogLevel::Normal,
                uint64_t tick = 0)
      : mData{data}, mId{id}, mCachedAgentIdStr{std::to_string(id)} {
    // Log the initial state of the agent for replay purposes (not sure if this
    // is needed but seems useful to have the initial state in the log)
    mStates.push_back({
        {"agentId", mCachedAgentIdStr},
        {"actionType", "initial_state"},
        {"logLevel", static_cast<int>(logLevel)},
        {"timestamp", tick},
        {"details", data.ToJSON()}
    });
  }
  virtual ~StepAgentBase() = default;

  /// Get the ID of the agent
  [[nodiscard]] size_t getId() const noexcept { return mId; }

  [[nodiscard]] const std::vector<nlohmann::json>& GetStates() const {
    return mStates;
  }

  void loadFromJson(const nlohmann::json& eventData) {
    if (!eventData.contains("actionType") ||
        !eventData.at("actionType").is_string()) {
      return;
    }
    if (eventData.at("actionType").get<std::string>() != "movement" &&
        eventData.at("actionType").get<std::string>() != "initial_state") {
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

    if (!eventData.contains("details") ||
        !eventData.at("details").is_object()) {
      return;
    }

    if (!mIsReplay) {
      mIsReplay = true;
      mStates.clear(); // Clear the default initial_state from the constructor
    }
    mStates.push_back(eventData);

    // Apply initial state immediately so the agent starts correctly
    if (eventData.at("actionType").get<std::string>() == "initial_state") {
      mData = DataClass::FromJSON(eventData.at("details"));
    }
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
  // In regular usage, the world will need to pass the tick and log level for
  // logging purposes.
  void SetState(DataClass data, LogLevel logLevel = LogLevel::Normal,
                uint64_t tick = 0) {
    if (mIsReplay) {
      // Find the associated state in mStates for this tick,
      // did it in reverse to avoid writing duplicate info.
      for (auto it = mStates.rbegin(); it != mStates.rend(); ++it) {
        if (it->contains("timestamp") && it->at("timestamp") == tick) {
          if (it->contains("details") && it->at("details").is_object()) {
            mData = DataClass::FromJSON(it->at("details"));
          }
          break;
        }
      }
      return; // Do not log or overwrite states during replay
    }

    mData = data;
    // Here handle logic to log for replay?
    mStates.push_back({
        {"agentId", mCachedAgentIdStr},
        {"actionType", "movement"},
        {"logLevel", static_cast<int>(logLevel)},
        {"timestamp", tick},
        {"details", data.ToJSON()}
    });
  }

  // SetStateNoLog is used in cases where we want to update the agent's state
  // without logging an action. Useful for helper functions or when the world
  // needs to update the agent's state without it being considered an action
  // taken by the agent.
  void SetStateNoLog(DataClass data) { 
    if (mIsReplay) return;
    mData = data; 
  }

  virtual void SetGoal(WorldPosition position) = 0;
};

};  // namespace cse498
