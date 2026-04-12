#include "Parser.hpp"
#include "Interpreter/ast.hpp"
#include <expected>
#include <memory>
#include <vector>

namespace cse498 {

std::expected<void, ParseErr> Parser::parse(std::istream &in) {
  m_Nodes.clear();
  auto tokenize_res = m_Lexer.Tokenize(in);
  if (!tokenize_res.has_value()) {
    // Figure out what to do with a LexerErr
    return std::unexpected(
        ParseErr(ParseErr::TODO, tokenize_res.error().m_Msg));
  }

  while (m_Lexer.Any()) {
    auto stmt_res = parse_stmt();
    if (!stmt_res.has_value()) {
      // Figure out what to do with a LexerErr
      return std::unexpected(
          ParseErr(ParseErr::TODO, tokenize_res.error().m_Msg));
    }

    m_Nodes.emplace_back(std::move(stmt_res.value()));
  }

  // Interpreter will steal m_Nodes and m_Syms rather than returning them

  return {};
}

}; // namespace cse498
