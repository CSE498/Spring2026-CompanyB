#pragma once

#include "lexer-gen.hpp"
#include <expected>
#include <string>

namespace cse498::AgentLexer {

struct LexerErr {
  enum Kind {};

  Kind m_Kind;
  std::string m_Msg;

  LexerErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  LexerErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg("") {}
};

using Token = emplex::Token;

struct Lexer {};

}; // namespace cse498::AgentLexer
