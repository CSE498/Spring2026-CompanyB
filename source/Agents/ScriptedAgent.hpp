/**
 * Demo Scripting Module
 **/

#pragma once

#include "Interpreter/Evaluation/OpVisits.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"
#include "StateGridPosition.hpp"
#include "core/AgentData.hpp"
#include "core/Step.hpp"
#include "core/StepAgentBase.hpp"
#include "core/WorldPosition.hpp"
#include "core/core.hpp"
#include <any>
#include <variant>

#include <memory>

namespace cse498 {

using AST::Node;
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
  std::expected<Type, InterpErr> Visit(AST::ValMagic &);

  std::expected<Type, InterpErr> Evaluate(AST::Node &node) {
    return node.Accept(*this);
  }

  template <IsDataClass DataClass> AgentWrapper(ScriptedAgent<DataClass> *i) {
    if constexpr (std::is_same_v<DataClass, TrafficData>) {
      m_Env = Env::TRAFFIC;
    } else {
      m_Env = Env::INFECTION;
    }

    m_AgentPtr = std::make_any<ScriptedAgent<DataClass> *>(i);
  }
};

template <IsDataClass DataClass>
class ScriptedAgent : public StepAgentBase<DataClass> {

  // TODO: Actuall fill these out
  std::unique_ptr<Node> mInit;
  std::unique_ptr<Node> mTurn;

  std::unique_ptr<AgentWrapper> mAgentWrapper;

  StepContainer mCurrentTurn;

public:
  ScriptedAgent(DataClass initial_state, size_t id)
      : StepAgentBase<DataClass>(initial_state, id),
        mAgentWrapper(std::make_unique<AgentWrapper>(this)) {}

  ~ScriptedAgent() = default;

  ScriptedAgent &SetInit(std::unique_ptr<Node> init) {
    mInit = std::move(init);
    return *this;
  }

  ScriptedAgent &SetTurn(std::unique_ptr<Node> turn) {
    mTurn = std::move(turn);
    return *this;
  }

  /// Choose the action to take a step in the appropriate direction.
  StepContainer GetTurn() override {
    mCurrentTurn = StepContainer{}; // Clear the container

    auto res = mAgentWrapper->Evaluate(*mTurn);
    if (!res.has_value()) {
      // TODO we do no move, but we need to report error
      std::cout << res.error().ToStr();
      // std::terminate();
      return StepContainer{};
    };

    return std::move(mCurrentTurn);
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
    // Assignee is a symbol
    TRY_DECL(expr, node.m_Value->Accept(*mAgentWrapper));

    if (std::holds_alternative<std::shared_ptr<SymInfo>>(node.m_Sym)) {
      std::shared_ptr<SymInfo> sym =
          std::get<std::shared_ptr<SymInfo>>(node.m_Sym);

      // If expr is not same type as variable. TODO Perhpas add casting later?
      if (sym->type.index() != expr.index()) {
        return RuntimeErr(
            RuntimeErr::TYPE_MISMATCH,
            std::format("Attempted to assign expression of type {} "
                        "to variable of type {}",
                        TypeVariantToName(expr), TypeVariantToName(sym->type)));
        sym->type = expr;
      }
    } else {
      // Assignee is a magic val
      AST::ValMagic::Value val_magic =
          std::get<AST::ValMagic::Value>(node.m_Sym);

      switch (val_magic) {
      case AST::ValMagic::Value::POSITION: {
        if (!std::holds_alternative<Point>(expr))
          return RuntimeErr(RuntimeErr::TYPE_MISMATCH,
                            std::format("Attempted to set magic value "
                                        "__position__ to non-point type '{}'",
                                        TypeVariantToName(expr)));
        auto data = this->GetState();
        data.position = std::get<Point>(expr);
        this->SetState(data);
      };
      default:
        return RuntimeErr(RuntimeErr::MAGIC_ERR,
                          "Attempted to set non-mutable magic value");
      };
    }

    return expr;
  }
  std::expected<Type, InterpErr> Visit(AST::StmtAgentDef &node) {
    return RuntimeErr{
        RuntimeErr::ENCOUNTERED_AGENT_DEF,
        "Encountered an agent definition when evaluation agent turn"};
  }

  std::expected<Type, InterpErr> Visit(AST::StmtAction &node) {
    TRY_DECL(type, node.m_Direction->Accept(*mAgentWrapper))
    if (Dir *direction = std::get_if<Dir>(&type)) {
      auto pos = this->GetState().position;
      switch (*direction) {
      case Dir::LEFT:
        pos = pos.Left();
        break;
      case Dir::RIGHT:
        pos = pos.Right();
        break;
      case Dir::UP:
        pos = pos.Up();
        break;
      case Dir::DOWN:
        pos = pos.Down();
        break;
      }
      if (mCurrentTurn.empty()) {
        mCurrentTurn.add_step(MovementStep{pos});
      } else {
        return RuntimeErr{RuntimeErr::TOO_MANY_MOVES,
                          "A maximum of 1 move is permitted per agent turn"};
      }
    } else {
      return RuntimeErr{RuntimeErr::INVALID_MOVE_ARG,
                        std::format("move() expects type 'Direction', found {}",
                                    TypeVariantToName(type))};
    }

    return NullType{};
  }

  std::expected<Type, InterpErr> Visit(AST::StmtLoopCtl &node) {
    if (node.m_Action == AST::StmtLoopCtl::BREAK) {
      return LoopControlErr(LoopControlErr::BREAK,
                            "'break' statement encountered outside of loop");
    }

    return LoopControlErr(LoopControlErr::CONTINUE,
                          "'continue' statement encountered outside of loop");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtWhile &node) {
    TRY_DECL(cond, node.m_Condition->Accept(*mAgentWrapper))
    TRY_DECL(cond_truthy, evaluate_bool(cond))

    while (cond_truthy) {
      auto body_res = node.m_Body->Accept(*mAgentWrapper);
      if (!body_res.has_value()) {
        if (auto *err = std::get_if<LoopControlErr>(&body_res.error())) {
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
  std::expected<Type, InterpErr> Visit(AST::ValMagic &node) {
    if (!node.m_Getting)
      return RuntimeErr(RuntimeErr::MAGIC_ERR, "ValMagic visited when setting");
    // Non world-specific values
    using Value = AST::ValMagic::Value;
    if (node.m_Value == Value::POSITION)
      return this->GetState().position;
    if (node.m_Value == Value::DESTINATION)
      return this->GetState().destination.value();

    if constexpr (std::is_same_v<DataClass, DiseaseData>) {
      // Disease world magic values
      auto data = this->GetState();

      switch (node.m_Value) {
      case Value::INFECTED:
        return data.infection_state == HealthState::INFECTED;
      case Value::SUSCEPTIBLE:
        return data.infection_state == HealthState::SUSCEPTIBLE;
      case Value::RECOVERED:
        return data.infection_state == HealthState::RECOVERED;
      default:
        break;
      };

    } else {
      // Traffic world magic values
      TrafficData data = this->GetState();

      switch (node.m_Value) {
      case Value::FACING:
        switch (data.direction) {
        case Direction::North:
          return Dir::UP;
        case Direction::East:
          return Dir::RIGHT;
        case Direction::South:
          return Dir::DOWN;
        case Direction::West:
          return Dir::LEFT;
        };
        break;
      default:
        break;
      };
    }

    // If we're here, the value requested is not valid in this dataclass
    return RuntimeErr(
        RuntimeErr::MAGIC_ERR,
        "Could not match requested magic value to world dataclass for setting");
  }

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
} // End of namespace cse498
