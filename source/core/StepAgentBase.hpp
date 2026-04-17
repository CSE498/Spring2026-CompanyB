#pragma once

#include "Step.hpp"
#include "WorldPosition.hpp"
#include "core.hpp"

namespace cse498 {
using Concepts::IsDataClass;
using cse498::steps::StepContainer;

template <IsDataClass DataClass>
class StepAgentBase {
  /// Unique id of the agent
  const size_t mId;

  /// This agent is replaying a previous game, so override any setState calls with the replayed states
  bool mIsReplaying = false;

  /// Index for replaying state history
  size_t mReplayIndex = 0;

 protected:
  /// Data class that holds agent data
  DataClass mData;

  /// A history of the agent's state after each turn, for logging/replay
  std::vector<nlohmann::json> mStateHistory;

 public:
  StepAgentBase(DataClass data, size_t id) : mData{data}, mId{id} {}
  virtual ~StepAgentBase() = default;

  /// Get the ID of the agent
  [[nodiscard]] size_t GetId() const noexcept { return mId; }

  // The main logic that separates the agents. When prompted for their turn,
  // this where the descisions are made
  [[nodiscard]] virtual StepContainer GetTurn() = 0;

  /*
   * Returning a copy is (most likely) correct here since DataClass is most
   * likely cheap to copy and we dont to modify the DataClass without passing
   * through SetState() since thats what logs the state changes for replay (?)
   */
  [[nodiscard]] DataClass GetState() const noexcept { return mData; }

  void SetState(DataClass data) {
    if (!mIsReplaying) {
      mStateHistory.push_back(mData.ToJson());
      mData = data;
    } else {
      // Use mReplayIndex to track replay position
      if (mReplayIndex < mStateHistory.size()) {
        mData.FromJson(mStateHistory[mReplayIndex]);
        ++mReplayIndex;
      }
    }
  }

  virtual void SetGoal(WorldPosition position) = 0;

  /// @brief Get a JSON representation of the agent's history of states, for logging
  virtual std::vector<nlohmann::json> GetStates() const { return mStateHistory; }

  virtual void SetStates(const std::vector<nlohmann::json>& states) { 
    mStateHistory = states;
    mIsReplaying = true;
    mReplayIndex = 0;
  }
};

};  // namespace cse498
