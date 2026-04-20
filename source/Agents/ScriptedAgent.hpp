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
#include <ranges>
#include <variant>

#include <memory>

namespace cse498 {

using AST::Node;
using Concepts::IsDataClass;
using namespace agentlang::Types;
using namespace agentlang::Symbols;
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
  std::expected<Type, InterpErr> Visit(AST::StmtReturn &);
  std::expected<Type, InterpErr> Visit(AST::StmtFuncCall &);
  std::expected<Type, InterpErr> Visit(AST::StmtFunc &);
  std::expected<Type, InterpErr> Visit(AST::ValLiteral &);
  std::expected<Type, InterpErr> Visit(AST::ValVariable &);

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

  std::optional<Type> mCurrentRetval = {};

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

  /* --------------- Visits for symbol types ---------------- */
  struct SymAssignVisitor {
    std::unique_ptr<AgentWrapper> &mAgentWrapper;
    std::unique_ptr<Node> &mVal;
    std::shared_ptr<SymInfo> &mSymPtr;
    StepAgentBase<DataClass> &mAgentBase;

    SymAssignVisitor(std::unique_ptr<AgentWrapper> &_AgentWrapper,
                     std::unique_ptr<Node> &_Val,
                     std::shared_ptr<SymInfo> &_SymPtr,
                     StepAgentBase<DataClass> &_AgentBase)
        : mAgentWrapper(_AgentWrapper), mVal(_Val), mSymPtr(_SymPtr),
          mAgentBase(_AgentBase) {};

    std::expected<Type, InterpErr> operator()(VarSym v) {
      TRY_DECL(val_result, mVal->Accept(*mAgentWrapper));

      // Handle for type mismatch
      if (v.m_Type.index() != val_result.index())
        return RuntimeErr(
            RuntimeErr::TYPE_MISMATCH,
            std::format("Attempted to assign expression of type {} "
                        "to variable of type {}",
                        TypeVariantToName(val_result),
                        TypeVariantToName(v.m_Type)));

      mSymPtr->sym = VarSym(val_result);
      return val_result;
    }

    std::expected<Type, InterpErr> operator()(MagicSym m) {
      // Assigning mVal's result to an agent value
      using Value = MagicSym::Value;

      TRY_DECL(val_result, mVal->Accept(*mAgentWrapper));

      switch (m.m_Value) {
      case Value::DESTINATION: {
        // Ensure val_result is a point
        if (!std::holds_alternative<Point>(val_result))
          return RuntimeErr(
              RuntimeErr::TYPE_MISMATCH,
              std::format("Attempted to set magic value "
                          "__destination__ to non-point type '{}'",
                          TypeVariantToName(val_result)));

        auto data = mAgentBase.GetState();
        data.position = std::get<Point>(val_result);
        mAgentBase.SetState(data);
        break;
      }
      default:
        return RuntimeErr(RuntimeErr::MAGIC_ERR,
                          "Attempted to set non-mutable magic value");
      };

      return val_result;
    }

    std::expected<Type, InterpErr> operator()(FuncSym f) {
      return RuntimeErr(RuntimeErr::IMMUTABLE_ERR,
                        "Attempted to assign to function");
    }
  };

  struct SymGetVisitor {
    std::unique_ptr<AgentWrapper> &mAgentWrapper;
    StepAgentBase<DataClass> &mAgentBase;
    std::vector<Type> mParams{};

    SymGetVisitor(std::unique_ptr<AgentWrapper> &_AgentWrapper,
                  StepAgentBase<DataClass> &_AgentBase,
                  std::vector<Type> _Params)
        : mAgentWrapper(_AgentWrapper), mAgentBase(_AgentBase),
          mParams(_Params) {}

    std::expected<Type, InterpErr> operator()(VarSym v) { return v.m_Type; }

    std::expected<Type, InterpErr> operator()(MagicSym m) {
      using Value = MagicSym::Value;
      auto data = mAgentBase.GetState();

      switch (m.m_Value) {
      case MagicSym::Value::POSITION:
        return Type{data.position};
      case MagicSym::Value::DESTINATION: {
        auto ret = data.destination;
        if (!ret.has_value())
          return NullType{};
        else
          return Type{ret.value()};
      }
      case MagicSym::Value::INFECTED:
        if constexpr (std::is_same_v<DataClass, DiseaseData>) {
          return Type{data.infection_state == HealthState::INFECTED};
        }
        break;
      case MagicSym::Value::SUSCEPTIBLE:
        if constexpr (std::is_same_v<DataClass, DiseaseData>) {
          return Type{data.infection_state == HealthState::SUSCEPTIBLE};
        }
        break;
      case MagicSym::Value::RECOVERED:
        if constexpr (std::is_same_v<DataClass, DiseaseData>) {
          return Type{data.infection_state == HealthState::RECOVERED};
        }
        break;
      case MagicSym::Value::FACING:
        if constexpr (std::is_same_v<DataClass, TrafficData>) {
          switch (data.direction) {
          case Direction::North:
            return Type{Dir::UP};
          case Direction::East:
            return Type{Dir::RIGHT};
          case Direction::South:
            return Type{Dir::DOWN};
          case Direction::West:
            return Type{Dir::LEFT};
          };
        }
        break;
      }
      // If we're here, the value requested is not valid in this dataclass
      return RuntimeErr(RuntimeErr::MAGIC_ERR,
                        "Could not match requested magic value to world "
                        "dataclass for setting");
    }

    std::expected<Type, InterpErr> operator()(FuncSym f) {
      // Sequentially set each param symbol
      for (auto [index, val] : std::views::enumerate(mParams)) {
        f.m_Params[index]->sym = val;
      }
      return NullType{};
    }
  };
  /* -------------------------------------------------------- */

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

    if (!node.m_Sym->mut)
      return RuntimeErr(RuntimeErr::IMMUTABLE_ERR,
                        std::format("Tried to modify immutable symbol '{}:{}'",
                                    node.m_Sym->sym.StateAsStr(),
                                    node.m_Sym->name));

    return std::visit(
        SymAssignVisitor(mAgentWrapper, node.m_Value, node.m_Sym, *this),
        node.m_Sym->sym);
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
  std::expected<Type, InterpErr> Visit(AST::StmtFunc &node) {
    return RuntimeErr(RuntimeErr::IMPOSSIBLE_STATE,
                      "Function definition node was visited");
  }
  std::expected<Type, InterpErr> Visit(AST::StmtReturn &node) {
    TRY_DECL(retval, node.m_Value->Accept(*mAgentWrapper));
    mCurrentRetval = retval;
    return LoopControlErr(LoopControlErr::RETURN);
  }
  std::expected<Type, InterpErr> Visit(AST::StmtFuncCall &node) {
    if (!node.m_Body.has_value())
      return RuntimeErr(RuntimeErr::UNRESOLVED_FUNCTION,
                        "Function has lingering unresolved call");

    if (!std::holds_alternative<FuncSym>(node.m_Symbol->sym))
      return RuntimeErr(RuntimeErr::UNRESOLVED_FUNCTION,
                        "Function has invalid symbol");

    FuncSym f = std::get<FuncSym>(node.m_Symbol->sym);

    if (f.m_Params.size() < node.m_Args.size())
      return RuntimeErr(RuntimeErr::TOO_FEW_ARGS);

    else if (f.m_Params.size() > node.m_Args.size())
      return RuntimeErr(RuntimeErr::TOO_MANY_ARGS);

    // Set args
    for (auto [index, arg] : std::views::enumerate(node.m_Args)) {
      auto &param = f.m_Params.at(index);
      if (!std::holds_alternative<VarSym>(param->sym))
        return RuntimeErr(RuntimeErr::IMPOSSIBLE_STATE,
                          "Function parameter has non-variable symbol type");

      TRY(arg->Accept(*mAgentWrapper));
    }

    // RIGHT HERE
    std::expected<Type, InterpErr> func_res =
        node.m_Body.value()->Accept(*mAgentWrapper);

    if (!func_res.has_value() &&
        (func_res.error().Is<LoopControlErr>(LoopControlErr::RETURN))) {
      assert(mCurrentRetval.has_value());
      auto retval_tmp = mCurrentRetval.value();
      mCurrentRetval = {};
      return retval_tmp;
    } else if (!func_res.has_value()) {
      return func_res.error();
    } else {
      return RuntimeErr(RuntimeErr::MISSING_RETURN);
    }
  }
  std::expected<Type, InterpErr> Visit(AST::ValLiteral &node) {
    return node.m_Val;
  }
  std::expected<Type, InterpErr> Visit(AST::ValVariable &node) {
    return std::visit(SymGetVisitor(mAgentWrapper, *this, {}),
                      node.m_Symbol->sym);
  }

  void SetGoal([[maybe_unused]] WorldPosition pos) override {}
};
// clang-format on
} // End of namespace cse498
