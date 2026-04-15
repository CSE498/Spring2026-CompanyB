#include "Interpreter/Parser.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include <expected>
#include <memory>
#include <vector>

namespace cse498 {

std::expected<void, InterpErr> Parser::parse(std::istream &in) {
  m_Nodes.clear();

  auto tokenize_res = m_Lexer.Tokenize(in);
  if (!tokenize_res.has_value())
    return tokenize_res.error();
  // return std::unexpected(tokenize_res.error());

  while (m_Lexer.Any()) {
    auto stmt_res = parse_stmt();
    if (!stmt_res.has_value())
      stmt_res.error();

    m_Nodes.emplace_back(std::move(stmt_res.value()));
  }

  // Interpreter will steal m_Nodes and m_Syms rather than returning them

  return {};
}

std::expected<AgentLexer::Token, InterpErr> Parser::parse_type() {
  using AgentLexer::IDs;
  return m_Lexer.UseIf(IDs::ID_KW_BOOL, IDs::ID_KW_INT, IDs::ID_KW_DOUBLE,
                       IDs::ID_KW_DIRECTION_T, IDs::ID_KW_CAR,
                       IDs::ID_KW_STUDENT);
}
}; // namespace cse498
