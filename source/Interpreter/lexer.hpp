#pragma once

#include "core.hpp"
#include "lexer-gen.hpp"
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

namespace cse498::AgentLexer {

struct LexerErr {
  enum Kind {
    UNEXP_TOKEN,
  };

  Kind m_Kind;
  std::string m_Msg;

  LexerErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  LexerErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg("") {}
};

using Token = emplex::Token;

class Lexer {
  emplex::Lexer m_Lexer{};

public:
  std::expected<void, LexerErr> Tokenize(std::string_view);
  std::expected<void, LexerErr> Tokenize(std::istream &);

  bool Any() const { return m_Lexer.token_id < m_Lexer.tokens.size(); }
  bool None() const { return m_Lexer.token_id >= m_Lexer.tokens.size(); };
  bool Is(std::convertible_to<int> auto... id) const {
    return (Any() && (... || (Peek().id == id)));
  }

  std::expected<Token, LexerErr> Use() {
    if (None())
      return m_Lexer.eof_token;

    return m_Lexer.tokens.at(m_Lexer.token_id++);
  }
  std::expected<Token, LexerErr> UseIf(std::convertible_to<int> auto... id) {
    if (!Is(id...)) {
      return std::unexpected(LexerErr(
          LexerErr::UNEXP_TOKEN,
          std::format("Expected one of [{}] but got {}",
                      m_Lexer.TokenName(id)..., m_Lexer.TokenName(Peek().id))));
    }
    return Use();
  }
  void Rewind(size_t steps = 1) {
    if (m_Lexer.token_id >= steps)
      m_Lexer.token_id -= steps;
    else
      m_Lexer.token_id = 0;
  }

  const Token &Peek(size_t skip_count = 0) const;
};

}; // namespace cse498::AgentLexer
