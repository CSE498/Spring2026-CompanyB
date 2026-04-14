#pragma once
#include "core.hpp"
#include <string>
namespace cse498 {

struct ASTErr {
  enum Kind {
    UNTYPED_NODE,
  };

  Kind kind_;
  std::string why_;

  ASTErr(Kind kind) : kind_(kind) {};
  ASTErr(Kind kind, std::string const &why) : kind_(kind), why_(why) {};
};

struct LexerErr {
  enum Kind {
    UNEXP_TOKEN,
  };

  Kind m_Kind;
  std::string m_Msg;

  LexerErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  LexerErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg("") {}
};

struct ParseErr {
  enum Kind {
    MISSING_TOKEN,
    EXPECTED_STMT,
    OUT_OF_TURN,
    OUT_OF_INIT,
    OUT_OF_LOOP,
    TODO,
  };

  Kind m_Kind;
  std::string m_Msg;
  ParseErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  ParseErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg(msg) {}
};

struct SymbolErr {
  enum Kind {
    UNDEFINED_SYMBOL,
    REDEFINITION,
    INVALID_TYPE,
  };

  Kind m_Kind;
  std::string m_Msg;
  SymbolErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  SymbolErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg(msg) {}
};

struct RuntimeErr {
  enum Kind {
    TYPE_MISMATCH,
  };

  Kind m_Kind;
  std::string m_Msg;
  RuntimeErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  RuntimeErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg(msg) {}
};

using InterpErr = std::variant<LexerErr, ParseErr, SymbolErr, ASTErr>;

}; // namespace cse498
