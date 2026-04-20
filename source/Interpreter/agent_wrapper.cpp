#include "Agents/ScriptedAgent.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "core/AgentData.hpp"

namespace cse498 {

std::expected<Type, InterpErr>
templ_visit(AgentWrapper &i, std::derived_from<AST::Node> auto &node) {
  if (!i.m_AgentPtr.has_value())
    return RuntimeErr(
        RuntimeErr::EMPTY_INTERP_WRAPPER,
        "AgentWrapper is missing a pointer to the scripted agent");

  if (i.m_Env == AgentWrapper::Env::TRAFFIC) {
    return std::any_cast<ScriptedAgent<TrafficData> *>(i.m_AgentPtr)
        ->Visit(node);
  } else {
    return std::any_cast<ScriptedAgent<DiseaseData> *>(i.m_AgentPtr)
        ->Visit(node);
  }
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtBlock &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::ExprUnary &node) {
  return templ_visit(*this, node);
}
std::expected<Type, InterpErr> AgentWrapper::Visit(AST::ExprBinary &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::Assign &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtAgentDef &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtAction &node) {

  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtWhile &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtLoopCtl &node) {

  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtIf &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtReturn &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::ValLiteral &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtFunc &node) {
  return templ_visit(*this, node);
}
std::expected<Type, InterpErr> AgentWrapper::Visit(AST::StmtFuncCall &node) {
  return templ_visit(*this, node);
}

std::expected<Type, InterpErr> AgentWrapper::Visit(AST::ValVariable &node) {
  return templ_visit(*this, node);
}

}; // namespace cse498
