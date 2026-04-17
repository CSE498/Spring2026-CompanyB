#include "ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/interpreter.hpp"
#include <expected>
namespace cse498::AST {
Node::~Node() {};
TypedNode::~TypedNode() {};

std::expected<Types::Type, InterpErr> StmtBlock::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> ExprUnary::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
ExprBinary::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> Assign::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
StmtAgentDef::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
StmtAction::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtWhile::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
StmtLoopCtl::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr> StmtIf::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
ValLiteral::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

std::expected<Types::Type, InterpErr>
ValVariable::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

}; // namespace cse498::AST
