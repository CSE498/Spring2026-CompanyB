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

  using OpFuncUnary = std::function<std::expected<Type, InterpErr>(Type)>;
  using OpFuncBinary =
      std::function<std::expected<Type, InterpErr>(Type, Type)>;

  OpVisits<Dataclass> m_OpVisits{};
  RobinHoodMap<int, OpFuncUnary> m_OpDispUnary{
      {emplex::Lexer::ID_CMP_GEQ, OpVisits<Dataclass>::m_OpVisit_Un_GEQ},
      {emplex::Lexer::ID_CMP_GT, OpVisits<Dataclass>::m_OpVisit_Un_GT},
      {emplex::Lexer::ID_CMP_LEQ, OpVisits<Dataclass>::m_OpVisit_Un_LEQ},
      {emplex::Lexer::ID_CMP_LT, OpVisits<Dataclass>::m_OpVisit_Un_LT},
      {emplex::Lexer::ID_CMP_NEQ, OpVisits<Dataclass>::m_OpVisit_Un_NEQ},
      {emplex::Lexer::ID_CMP_EQ, OpVisits<Dataclass>::m_OpVisit_Un_EQ},
      {emplex::Lexer::ID_OP_REM, OpVisits<Dataclass>::m_OpVisit_Un_REM},
      {emplex::Lexer::ID_OP_ADD, OpVisits<Dataclass>::m_OpVisit_Un_ADD},
      {emplex::Lexer::ID_OP_DIVIDE, OpVisits<Dataclass>::m_OpVisit_Un_DIVIDE},
      {emplex::Lexer::ID_OP_MULT, OpVisits<Dataclass>::m_OpVisit_Un_MULT},
      {emplex::Lexer::ID_OP_LNOT, OpVisits<Dataclass>::m_OpVisit_Un_LNOT},
      {emplex::Lexer::ID_OP_MINUS, OpVisits<Dataclass>::m_OpVisit_Un_MINUS},
  };
  RobinHoodMap<int, OpFuncBinary> m_OpDispBinary{
      {emplex::Lexer::ID_CMP_GEQ, OpVisits<Dataclass>::m_OpVisit_Bi_GEQ},
      {emplex::Lexer::ID_CMP_GT, OpVisits<Dataclass>::m_OpVisit_Bi_GT},
      {emplex::Lexer::ID_CMP_LEQ, OpVisits<Dataclass>::m_OpVisit_Bi_LEQ},
      {emplex::Lexer::ID_CMP_LT, OpVisits<Dataclass>::m_OpVisit_Bi_LT},
      {emplex::Lexer::ID_CMP_NEQ, OpVisits<Dataclass>::m_OpVisit_Bi_NEQ},
      {emplex::Lexer::ID_CMP_EQ, OpVisits<Dataclass>::m_OpVisit_Bi_EQ},
      {emplex::Lexer::ID_OP_REM, OpVisits<Dataclass>::m_OpVisit_Bi_REM},
      {emplex::Lexer::ID_OP_ADD, OpVisits<Dataclass>::m_OpVisit_Bi_ADD},
      {emplex::Lexer::ID_OP_DIVIDE, OpVisits<Dataclass>::m_OpVisit_Bi_DIVIDE},
      {emplex::Lexer::ID_OP_MULT, OpVisits<Dataclass>::m_OpVisit_Bi_MULT},
      {emplex::Lexer::ID_OP_MINUS, OpVisits<Dataclass>::m_OpVisit_Bi_MINUS},
  };

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
    auto res = node.Accept(*m_InterpWrapper);
    if (!res.has_value())
      return res.error();

    auto func = m_OpDispUnary.at(node.m_Token.id);
    if (!func.has_value())
      return func.error();

    return std::visit(func.value(), res.value());
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
