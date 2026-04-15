#include "Interpreter/interpreter.hpp"

namespace cse498 {

std::expected<void, InterpErr>
templ_visit(InterpreterWrapper &i, std::derived_from<AST::Node> auto &node) {
  if (!i.m_InterpreterPtr.has_value())
    return RuntimeErr(
        RuntimeErr::EMPTY_INTERP_WRAPPER,
        "InterpreterWrapper is missing a pointer to the interpreter");

  if (i.m_Env == InterpreterWrapper::Env::TRAFFIC) {
    return std::any_cast<Interpreter<TrafficData> *>(i.m_InterpreterPtr)
        ->Visit(node);
  } else {
    return std::any_cast<Interpreter<InfectionData> *>(i.m_InterpreterPtr)
        ->Visit(node);
  }
}

std::expected<void, InterpErr> InterpreterWrapper::Visit(AST::StmtBlock &node) {
  return templ_visit(*this, node);
}

std::expected<void, InterpErr> InterpreterWrapper::Visit(AST::ExprUnary &node) {
  return templ_visit(*this, node);
}

std::expected<void, InterpErr> InterpreterWrapper::Visit(AST::Assign &node) {
  return templ_visit(*this, node);
}

std::expected<void, InterpErr>
InterpreterWrapper::Visit(AST::StmtAgentDef &node) {

  return templ_visit(*this, node);
}

std::expected<void, InterpErr>
InterpreterWrapper::Visit(AST::StmtAction &node) {

  return templ_visit(*this, node);
}

std::expected<void, InterpErr> InterpreterWrapper::Visit(AST::StmtWhile &node) {
  return templ_visit(*this, node);
}

std::expected<void, InterpErr>
InterpreterWrapper::Visit(AST::StmtLoopCtl &node) {

  return templ_visit(*this, node);
}

std::expected<void, InterpErr> InterpreterWrapper::Visit(AST::StmtIf &node) {

  return templ_visit(*this, node);
}

std::expected<void, InterpErr>
InterpreterWrapper::Visit(AST::ValLiteral &node) {

  return templ_visit(*this, node);
}

std::expected<void, InterpErr>
InterpreterWrapper::Visit(AST::ValVariable &node) {
  return templ_visit(*this, node);
}

}; // namespace cse498
