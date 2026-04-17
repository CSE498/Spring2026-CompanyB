/**
 * Demo Scripting Module
 **/

#pragma once

#include "core/Step.hpp"
#include "core/StepAgentBase.hpp"
#include "core/WorldPosition.hpp"
#include "core/core.hpp"
#include "Interpreter/ast.hpp"

#include <memory>

namespace cse498 {

using AST::StmtBlock;
using Concepts::IsDataClass;

template <IsDataClass DataClass>
class ScriptedAgent : public StepAgentBase<DataClass> {

  // TODO: Actuall fill these out
  std::unique_ptr<StmtBlock> mInit;
  std::unique_ptr<StmtBlock> mTurn;

public:
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id) {}
  ~ScriptedAgent() = default;

  void SetInit(std::unique_ptr<StmtBlock> init) {
    mInit = std::move(init);
  }

  void SetTurn(std::unique_ptr<StmtBlock> turn) {
    mTurn = std::move(turn);
  }

  /// Choose the action to take a step in the appropriate direction.
  StepContainer GetTurn() override {
    auto turn = EvaluateTurn();
    return turn;
  }

  StepContainer EvaluateTurn() {
    return StepContainer{};
  }

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
} // End of namespace cse498
