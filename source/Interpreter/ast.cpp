#include "ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/interpreter.hpp"
#include <expected>
namespace cse498::AST {
Node::~Node() {};

std::expected<void, InterpErr> StmtBlock::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> ExprUnary::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> ExprBinary::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> Assign::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> StmtAgentDef::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> StmtAction::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> StmtWhile::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> StmtLoopCtl::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> StmtIf::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}
std::expected<void, InterpErr> ValLiteral::Accept(InterpreterWrapper &i) {
  return i.Visit(*this);
}

}; // namespace cse498::AST
