/**
 * Demo Scripting Module
 **/

#pragma once

#define TRY_DECL(var, expr) \
  auto _result_##var = (expr); \
  if (!_result_##var.has_value()) { return _result_##var.error(); } \
  auto var = _result_##var.value();

#define TRY(expr) \
  { \
    auto _result = (expr); \
    if (!_result.has_value()) { return _result.error(); } \
  }

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
using steps::MovementStep;


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

  StepContainer mCurrentTurn;

public:
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id) {}
  ~ScriptedAgent() = default;

  ScriptedAgent& SetInit(std::unique_ptr<StmtBlock> init) { mInit = std::move(init); return *this; }

  ScriptedAgent& SetTurn(std::unique_ptr<StmtBlock> turn) { mTurn = std::move(turn); return *this; }

  /// Choose the action to take a step in the appropriate direction.
  StepContainer GetTurn() override {
    mCurrentTurn = StepContainer{}; // Clear the container

    if (!Visit(*mTurn).has_value()) {
      // TODO we do no move, but we need to report error
      return StepContainer{};
    };

    return mCurrentTurn;
  }


  std::expected<Type, InterpErr> Visit(AST::StmtBlock &node) {
    std::expected<Type, InterpErr> res;
    for (auto &cur_node : node.m_Body) {
      TRY(cur_node->Accept(*mAgentWrapper))
    }

    return NullType{};
  }
  std::expected<Type, InterpErr> Visit(AST::ExprUnary &node) {
    TRY_DECL(res, node.m_Left->Accept(*mAgentWrapper))

    return evaluate_unary(node.m_Token, res);
  }
  std::expected<Type, InterpErr> Visit(AST::ExprBinary &node) {
    TRY_DECL(lhs, node.m_Left->Accept(*mAgentWrapper))
    TRY_DECL(rhs, node.m_Right->Accept(*mAgentWrapper))

    return evaluate_binary(node.m_Token, lhs, rhs);
  }
  std::expected<Type, InterpErr> Visit(AST::Assign &node) {
    TRY_DECL(expr, node.m_Value->Accept(*mAgentWrapper));

    // If expr is not same type as variable. TODO Perhpas add casting later?
    if (node.m_Sym->type.index() != expr.index()) {
      return RuntimeErr(
        RuntimeErr::TYPE_MISMATCH, 
        std::format("Attempted to assign expression of type {} to variable of type {}",
          TypeVariantToName(expr),
          TypeVariantToName(node.m_Sym->type)
        )
      );
    }

    node.m_Sym->type = expr;
    return expr;
  }
  std::expected<Type, InterpErr> Visit(AST::StmtAgentDef &node) {
    return RuntimeErr{
      RuntimeErr::ENCOUNTERED_AGENT_DEF,
      "Encountered an agent definition when evaluation agent turn"
    };
  }

  std::expected<Type, InterpErr> Visit(AST::StmtAction &node) {
    TRY_DECL(type, node.m_Direction->Accept(*mAgentWrapper))
    if (Dir* direction = std::get_if<Dir>(&type)) {
      auto pos = this->GetState().position;
      switch (*direction) {
        case Dir::LEFT: pos = pos.Left();
        case Dir::RIGHT: pos = pos.Right();
        case Dir::UP: pos = pos.Up();
        case Dir::DOWN: pos = pos.Down();
      }
      if (mCurrentTurn.empty()) {
        mCurrentTurn.add_step(MovementStep{pos});
      } else {
        return RuntimeErr{
          RuntimeErr::TOO_MANY_MOVES,
          "A maximum of 1 move is permitted per agent turn"
        };
      }
    } else {
      return RuntimeErr{
        RuntimeErr::INVALID_MOVE_ARG,
        std::format("move() expects type 'Direction', found {}",
        TypeVariantToName(type)
        )
      };
    }

    return NullType{};
  }

  std::expected<Type, InterpErr> Visit(AST::StmtLoopCtl &node) {
    if (node.m_Action == AST::StmtLoopCtl::BREAK) {
      return LoopControlErr(
        LoopControlErr::BREAK,
        "'break' statement encountered outside of loop"
      );
    } 

    return LoopControlErr(
      LoopControlErr::CONTINUE,
      "'continue' statement encountered outside of loop"
    );
  }
  std::expected<Type, InterpErr> Visit(AST::StmtWhile &node) {
    TRY_DECL(cond, node.m_Condition->Accept(*mAgentWrapper))
    TRY_DECL(cond_truthy, evaluate_bool(cond))
    
    while (cond_truthy) {
      auto body_res = node.m_Body->Accept(*mAgentWrapper);
      if (!body_res.has_value()) {
        if (auto* err = std::get_if<LoopControlErr>(&body_res.error())) {
            if (err->m_Kind == LoopControlErr::BREAK) {
              break;
            } else {
              continue;
            }
        } else {
          return body_res.error();
        }
      }

      TRY_DECL(new_cond, node.m_Condition->Accept(*mAgentWrapper))
      TRY_DECL(new_cond_truthy, evaluate_bool(new_cond))
      cond_truthy = new_cond_truthy;
    }
    return NullType{};
  }
  std::expected<Type, InterpErr> Visit(AST::StmtIf &node) {
    TRY_DECL(cond, node.m_Condition->Accept(*mAgentWrapper))
    TRY_DECL(cond_truthy, evaluate_bool(cond))

    if (cond_truthy) {
      TRY(node.m_TBody->Accept(*mAgentWrapper))
    } else if (node.m_FBody.has_value()) {
      TRY(node.m_FBody.value()->Accept(*mAgentWrapper))
    }

    return NullType{};
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
