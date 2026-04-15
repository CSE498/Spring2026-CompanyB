#pragma once
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "agentlang.hpp"
#include "tools/RobinHoodMap.hpp"
#include <any>
#include <string>

namespace cse498 {
using namespace cse498::agentlang::Types;

template <typename... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};

template <typename T>
concept Numeric = Concepts::IsOneOf<int, double>;

// GET DATACLASSES IN THIS BRANCH SO ASAP, FOR NOW JUST DOING THIS TO DEMO
struct InfectionData {};
struct TrafficData {};

struct InterpreterWrapper;

template <typename Dataclass> class Interpreter {
public:
  std::expected<void, InterpErr> Visit(AST::StmtBlock &);
  std::expected<void, InterpErr> Visit(AST::ExprUnary &);
  std::expected<void, InterpErr> Visit(AST::Assign &);
  std::expected<void, InterpErr> Visit(AST::StmtAgentDef &);
  std::expected<void, InterpErr> Visit(AST::StmtAction &);
  std::expected<void, InterpErr> Visit(AST::StmtWhile &);
  std::expected<void, InterpErr> Visit(AST::StmtLoopCtl &);
  std::expected<void, InterpErr> Visit(AST::StmtIf &);
  std::expected<void, InterpErr> Visit(AST::ValLiteral &);
  std::expected<void, InterpErr> Visit(AST::ValVariable &);
};

// Pull interpreter up into an any
struct InterpreterWrapper {
  enum class Env { TRAFFIC, INFECTION };

  Env m_Env;
  std::any m_InterpreterPtr;

  std::expected<void, InterpErr>
  Visit(std::derived_from<AST::Node> auto &node) {
    if (!m_InterpreterPtr.has_value())
      return RuntimeErr(
          RuntimeErr::EMPTY_INTERP_WRAPPER,
          "InterpreterWrapper is missing a pointer to the interpreter");

    if (m_Env == Env::TRAFFIC) {
      return std::any_cast<Interpreter<TrafficData> *>(m_InterpreterPtr)
          ->Visit(node);
    } else {
      return std::any_cast<Interpreter<InfectionData> *>(m_InterpreterPtr)
          ->Visit(node);
    }
  }

  std::expected<void, InterpErr> Evaluate(std::unique_ptr<AST::Node> &node) {
    return node->Accept(*this);
  }

  template <typename Dataclass> InterpreterWrapper(Interpreter<Dataclass> *i) {
    if constexpr (std::is_same_v<Dataclass, InfectionData>) {
      m_Env = Env::INFECTION;
    } else {
      m_Env = Env::TRAFFIC;
    }

    m_InterpreterPtr = std::make_any(i);
  }
};

}; // namespace cse498
