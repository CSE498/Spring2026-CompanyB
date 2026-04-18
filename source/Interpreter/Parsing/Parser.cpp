#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include <expected>
#include <memory>
#include <vector>

namespace cse498 {

std::expected<std::vector<std::unique_ptr<AST::StmtAgentDef>>, InterpErr> Parser::parse(std::istream &in) {
  using AgentLexer::IDs;
  m_AgentDefs.clear();

  auto tokenize_res = m_Lexer.Tokenize(in);
  if (!tokenize_res.has_value())
    return tokenize_res.error();

  /*
  We first need to check that the first statement configures the world
  ---
  <KW_WORLD> <KW_TRAFFIC|KW_INFECTION>;
  ---
  */
  // Expect: <KW_WORLD>
  auto token_res = m_Lexer.UseIf(IDs::ID_KW_WORLD);
  if (!token_res.has_value())
    return token_res.error();

  // Expect: <KW_TRAFFIC|KW_INFECTION>
  token_res = m_Lexer.UseIf(IDs::ID_KW_INFECTION, IDs::ID_KW_TRAFFIC);
  if (!token_res.has_value())
    return token_res.error();

  switch (token_res.value()) {
  case IDs::ID_KW_TRAFFIC:
    m_Env = Env::TRAFFIC;
    break;
  case IDs::ID_KW_INFECTION:
    m_Env = Env::INFECTION;
    break;
  default:
    return ParseErr(ParseErr::INVALID_WORLD,
                    std::format("Token '{}' is not a valid world configuration",
                                AgentLexer::TokenName(token_res.value())));
  }

  // Expect: <;>
  token_res = m_Lexer.UseIf(IDs::ID_DELIM_SEMICLN);
  if (!token_res.has_value())
    return token_res.error();

  // Now we parse every remaining statement
  m_Syms.PushSymbolScope();
  while (m_Lexer.Any()) {
    auto stmt_res = parse_stmt();
    if (!stmt_res.has_value()) {
      // Just break if we're done, not a fatal error
      if (stmt_res.error().Is<ParseErr>(ParseErr::AT_EOF))
        break;
      return stmt_res.error();
    }

    m_Nodes.emplace_back(std::move(stmt_res.value()));
  }
  m_Syms.PopSymbolScope();

  // Interpreter will steal m_Nodes and m_Syms rather than returning them

  return std::move(m_AgentDefs);
}

std::expected<AgentLexer::Token, InterpErr> Parser::parse_type() {
  using AgentLexer::IDs;
  return m_Lexer.UseIf(IDs::ID_KW_BOOL, IDs::ID_KW_INT, IDs::ID_KW_DOUBLE,
                       IDs::ID_KW_DIRECTION_T, IDs::ID_KW_CAR,
                       IDs::ID_KW_STUDENT);
}
}; // namespace cse498
