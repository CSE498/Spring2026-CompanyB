#include "ast.hpp"

#include <expected>

#include "Agents/ScriptedAgent.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"
namespace cse498::AST {
Node::~Node() {};
TypedNode::~TypedNode() {};

std::expected<Types::Type, InterpErr> StmtBlock::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ExprUnary::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ExprBinary::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> Assign::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtAgentDef::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtAction::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtWhile::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtLoopCtl::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtIf::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}
std::expected<Types::Type, InterpErr> StmtReturn::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}
std::expected<Types::Type, InterpErr> StmtFunc::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}
std::expected<Types::Type, InterpErr> StmtFuncCall::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ValLiteral::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ValVariable::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

// -----
std::expected<void, InterpErr> protected_finalize(std::unique_ptr<Node> &node,
                                                  SymbolTable &s) {
  if (node.get() != nullptr) {
    TRY(node->Finalize(s));
  }

  return {};
}

std::expected<void, InterpErr> protected_finalize(std::shared_ptr<Node> &node,
                                                  SymbolTable &s) {
  if (node.get() != nullptr) {
    TRY(node->Finalize(s));
  }

  return {};
}

std::expected<void, InterpErr> StmtBlock::Finalize(SymbolTable &s) {
  for (auto &node : m_Body) {
    // TRY(node->Finalize(s));
    TRY(protected_finalize(node, s));
  }
  return {};
}

std::expected<void, InterpErr> ExprUnary::Finalize(SymbolTable &s) {
  // TRY(m_Left->Finalize(s));
  TRY(protected_finalize(m_Left, s));
  return {};
}

std::expected<void, InterpErr> ExprBinary::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Left, s));
  TRY(protected_finalize(m_Right, s));
  return {};
}

std::expected<void, InterpErr> Assign::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Value, s));
  return {};
}

std::expected<void, InterpErr> StmtAgentDef::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Init, s));
  TRY(protected_finalize(m_Turn, s));
  return {};
}

std::expected<void, InterpErr> StmtAction::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Direction, s));
  return {};
}

std::expected<void, InterpErr> StmtWhile::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Condition, s));
  TRY(protected_finalize(m_Body, s));
  return {};
}

std::expected<void, InterpErr> StmtLoopCtl::Finalize(
    [[maybe_unused]] SymbolTable &s) {
  return {};
}

std::expected<void, InterpErr> StmtIf::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Condition, s));
  TRY(protected_finalize(m_TBody, s));
  if (m_FBody.has_value()) {
    TRY(protected_finalize(m_FBody.value(), s));
  }
  return {};
}
std::expected<void, InterpErr> StmtReturn::Finalize(SymbolTable &s) {
  TRY(protected_finalize(m_Value, s));
  return {};
}
std::expected<void, InterpErr> StmtFunc::Finalize(
    [[maybe_unused]] SymbolTable &s) {
  return {};
}
std::expected<void, InterpErr> StmtFuncCall::Finalize(SymbolTable &s) {
  // All funcs should be resolved now
  FuncSym sym = std::get<FuncSym>(m_Symbol->sym);
  TRY_DECL(func_ptr, s.GetFunc(sym.m_BodyIdx));
  m_Body = func_ptr;
  TRY(protected_finalize(m_Body.value(), s));
  return {};
}

std::expected<void, InterpErr> ValLiteral::Finalize(
    [[maybe_unused]] SymbolTable &s) {
  return {};
}

std::expected<void, InterpErr> ValVariable::Finalize(
    [[maybe_unused]] SymbolTable &s) {
  return {};
}

};  // namespace cse498::AST
