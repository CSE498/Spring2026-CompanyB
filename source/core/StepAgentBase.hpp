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

 protected:
  /// Data class that holds agent data
  DataClass mData;

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
    mData = data;
    // Here handle logic to log for replay?
  }

  virtual void SetGoal(WorldPosition position) = 0;
};

};  // namespace cse498
