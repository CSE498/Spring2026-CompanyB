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

template <typename Dataclass> class Interpreter;

// Pull interpreter up into an any
struct InterpreterWrapper {
  enum class Env { TRAFFIC, INFECTION };

  Env m_Env;
  std::any m_InterpreterPtr;

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

template <typename Dataclass> class Interpreter {
  std::unique_ptr<InterpreterWrapper> m_InterpWrapper;

public:
  std::expected<void, InterpErr> Visit(AST::StmtBlock &node) {
    std::expected<void, InterpErr> res;
    for (auto &cur_node : node.m_Body) {
      res = cur_node->Accept(*m_InterpWrapper);
      if (!res.has_value())
        return res;
    }
  }
  std::expected<void, InterpErr> Visit(AST::ExprUnary &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::Assign &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::StmtAgentDef &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::StmtAction &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::StmtWhile &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::StmtLoopCtl &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::StmtIf &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::ValLiteral &node) {
    // TODO
  }
  std::expected<void, InterpErr> Visit(AST::ValVariable &node) {
    // TODO
  }
};

}; // namespace cse498
