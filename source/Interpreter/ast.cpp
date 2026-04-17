#include "ast.hpp"
#include "Agents/ScriptedAgent.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/interpreter.hpp"
#include <expected>
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

std::expected<Types::Type, InterpErr> ValLiteral::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ValVariable::Accept(AgentWrapper &i) {
  return i.Visit(*this);
}

}; // namespace cse498::AST
