#pragma once

#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

#include "Interpreter/errors.hpp"
#include "Lexing/lexer-gen.hpp"
#include "core.hpp"

namespace cse498::AgentLexer {

using Token = emplex::Token;
using IDs = emplex::Lexer;

/** @brief Retrieve the name of the token with ID `id`.
 */
[[nodiscard]] std::string inline TokenName(int id) {
  return emplex::Lexer::TokenName(id);
}

class Lexer {
  emplex::Lexer m_Lexer{};

 public:
  /** @brief Populate the lexer with the tokenization of `i`.
   */
  std::expected<void, InterpErr> Tokenize(std::string_view i) {
    m_Lexer.Tokenize(i);
    return {};
  };
  /** @brief Populate the lexer with the tokenization of `i`.
   */
  std::expected<void, InterpErr> Tokenize(std::istream& i) {
    m_Lexer.Tokenize(i);
    return {};
  };

  /** @brief Determine whether there are any tokens remaining.
   */
  bool Any() const { return m_Lexer.token_id < m_Lexer.tokens.size(); }
  /** @brief Determine whether the lexer is currently empty.
   */
  bool None() const { return m_Lexer.token_id >= m_Lexer.tokens.size(); };
  /** @brief Determine if the next token has any of the provided token IDs.
   */
  bool Is(std::convertible_to<int> auto... id) const {
    return (Any() && (... || (Peek().id == id)));
  }

  /** @brief Use the next token.
   */
  std::expected<Token, InterpErr> Use() {
    if (None())
      return ParseErr(ParseErr::AT_EOF,
                      "Reached EOF and tried to consume token");

    return m_Lexer.tokens.at(m_Lexer.token_id++);
  }
  /** @brief Use the next token only if it has any of the provided IDs,
   * otherwise return an `InterpErr` describing the error which occured.
   */
  std::expected<Token, InterpErr> UseIf(std::convertible_to<int> auto... id) {
    if (!Is(id...)) {
      // Build the expected tokens string by folding over the pack
      std::string expected;
      bool first = true;
      ((expected += (first ? "" : ", ") + m_Lexer.TokenName(id), first = false),
       ...);

      return LexerErr(LexerErr::UNEXP_TOKEN,
                      std::format("[at {}:{}] Expected one of [{}] but got {}",
                                  Peek().line_id, Peek().col_id, expected,
                                  m_Lexer.TokenName(Peek().id)));
    }
    return Use();
  }
  /** @brief Move the lexer back one or more tokens, but not further than the
   *   first token.
   *   @param steps The number of tokens to move backwards, defaults to 1.
   */
  void Rewind(size_t steps = 1) {
    if (m_Lexer.token_id >= steps)
      m_Lexer.token_id -= steps;
    else
      m_Lexer.token_id = 0;
  }

  const Token& Peek(size_t skip_count = 0) const {
    return m_Lexer.Peek(skip_count);
  };
};

};  // namespace cse498::AgentLexer
