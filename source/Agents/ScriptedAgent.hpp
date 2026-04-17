/**
 * Demo Scripting Module
 **/

#pragma once

#define TRY_DECL(var, expr) \
  auto _result_##var = (expr); \
  if (!_result_##var.has_value()) { return _result_##var.error(); } \
  auto var = _result_##var.value();

#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "core/DiseaseData.hpp"
#include "core/Step.hpp"
#include "core/StepAgentBase.hpp"
#include "core/WorldPosition.hpp"
#include "core/core.hpp"
#include <any>

#include <memory>

namespace cse498 {

using AST::StmtBlock;
using Concepts::IsDataClass;
using namespace agentlang::Types;

template <IsDataClass DataClass> class ScriptedAgent;

struct AgentWrapper {
  enum class Env { TRAFFIC, INFECTION };

  Env m_Env;
  std::any m_AgentPtr;

  std::expected<Type, InterpErr> Visit(AST::StmtBlock &);
  std::expected<Type, InterpErr> Visit(AST::ExprUnary &);
  std::expected<Type, InterpErr> Visit(AST::ExprBinary &);
  std::expected<Type, InterpErr> Visit(AST::Assign &);
  std::expected<Type, InterpErr> Visit(AST::StmtAgentDef &);
  std::expected<Type, InterpErr> Visit(AST::StmtAction &);
  std::expected<Type, InterpErr> Visit(AST::StmtWhile &);
  std::expected<Type, InterpErr> Visit(AST::StmtLoopCtl &);
  std::expected<Type, InterpErr> Visit(AST::StmtIf &);
  std::expected<Type, InterpErr> Visit(AST::ValLiteral &);
  std::expected<Type, InterpErr> Visit(AST::ValVariable &);

  std::expected<Type, InterpErr> Evaluate(std::unique_ptr<AST::Node> &node) {
    return node->Accept(*this);
  }

  template <IsDataClass DataClass> AgentWrapper(ScriptedAgent<DataClass> *i) {
    if constexpr (std::is_same_v<DataClass, DiseaseData>) {
      m_Env = Env::INFECTION;
    } else {
      m_Env = Env::TRAFFIC;
    }

    m_AgentPtr = std::make_any(i);
  }
};

template <IsDataClass DataClass>
class ScriptedAgent : public StepAgentBase<DataClass> {

  // TODO: Actuall fill these out
  std::unique_ptr<StmtBlock> mInit;
  std::unique_ptr<StmtBlock> mTurn;

  std::unique_ptr<AgentWrapper> mAgentWrapper;

public:
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id) {}
  ~ScriptedAgent() = default;

  void SetInit(std::unique_ptr<StmtBlock> init) { mInit = std::move(init); }

  void SetTurn(std::unique_ptr<StmtBlock> turn) { mTurn = std::move(turn); }

  /// Choose the action to take a step in the appropriate direction.
  StepContainer GetTurn() override {
    auto turn = EvaluateTurn();
    return turn;
  }

  StepContainer EvaluateTurn() { return StepContainer{}; }

  std::expected<Type, InterpErr> Visit(AST::StmtBlock &node) {
    std::expected<Type, InterpErr> res;
    for (auto &cur_node : node.m_Body) {
      res = cur_node->Accept(*mAgentWrapper);
      if (!res.has_value())
        return res;
    }

    return NullType{};
  }
  std::expected<Type, InterpErr> Visit(AST::ExprUnary &node) {
    auto res = node.m_Left->Accept(*mAgentWrapper);
    if (!res.has_value())
      return res.error();

    return evaluate_unary(node.m_Token, res.value());
  }
  std::expected<Type, InterpErr> Visit(AST::ExprBinary &node) {
    TRY_DECL(lhs, node.m_Left->Accept(*mAgentWrapper))
    TRY_DECL(rhs, node.m_Right->Accept(*mAgentWrapper))

    return evaluate_binary(node.m_Token, lhs, rhs);
  }
  std::expected<Type, InterpErr> Visit(AST::Assign &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtAgentDef &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtAction &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtWhile &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtLoopCtl &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtIf &node) {
    // auto cond = node.m_Condition->Accept(*mAgentWrapper);
    // if (!cond.has_value()) {
    //   return cond.error();
    // }

    // auto truthy = evaluate_bool(cond.value());
    // if (!truthy.has_value()) {
    //   return truthy.error();
    // }

    // if (truthy.value()) {
    //   auto do_if = node.m_TBody->Accept(*mAgentWrapper);
    // }
        return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::ValLiteral &node) {
    return node.m_Val;
  }
  std::expected<Type, InterpErr> Visit(AST::ValVariable &node) {
    return node.m_Symbol->type;
  }

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
} // End of namespace cse498
