#pragma once
#include "Evaluation/OpVisits.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "agentlang.hpp"
#include "core/DiseaseData.hpp"
#include "tools/RobinHoodMap.hpp"
#include <any>
#include <string>

namespace cse498 {
using namespace cse498::agentlang::Types;

template <typename T>
concept Numeric = Concepts::IsOneOf<int, double>;

template <typename Dataclass> class Interpreter;

// Pull interpreter up into an any
struct InterpreterWrapper {
  enum class Env { TRAFFIC, INFECTION };

  Env m_Env;
  std::any m_InterpreterPtr;

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

  template <typename Dataclass> InterpreterWrapper(Interpreter<Dataclass> *i) {
    if constexpr (std::is_same_v<Dataclass, DiseaseData>) {
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
  std::expected<Type, InterpErr> Visit(AST::StmtBlock &node) {
    std::expected<Type, InterpErr> res;
    for (auto &cur_node : node.m_Body) {
      res = cur_node->Accept(*m_InterpWrapper);
      if (!res.has_value())
        return res;
    }

    return NullType{};
  }
  std::expected<Type, InterpErr> Visit(AST::ExprUnary &node) {
    auto res = node.m_Left->Accept(*m_InterpWrapper);
    if (!res.has_value())
      return res.error();

    return evaluate_unary(node.m_Token, res.value());
  }
  std::expected<Type, InterpErr> Visit(AST::ExprBinary &node) {
    auto lhs = node.m_Left->Accept(*m_InterpWrapper);
    if (!lhs.has_value())
      return lhs.error();

    auto rhs = node.m_Right->Accept(*m_InterpWrapper);
    if (!rhs.has_value())
      return rhs.error();

    return evaluate_binary(node.m_Token, lhs.value(), rhs.value());
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
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::ValLiteral &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
  std::expected<Type, InterpErr> Visit(AST::ValVariable &node) {
    return TempErr(
        TempErr::NOT_IMPLEMENTED,
        "Don't yet know where and how Daniel intends to do evaluation");
  }
};

}; // namespace cse498
