#pragma once
#include <expected>
#include <string>
#include <utility>
#include <variant>
namespace cse498 {

struct BaseErr {
  std::string why_;

  BaseErr() = default;
  BaseErr(std::string const &why) : why_(why) {}

  template <typename T, typename E, typename Self>
  operator std::expected<T, E>(this Self &&self) {
    return std::unexpected(std::forward<Self>(self));
  }
};

struct ASTErr : BaseErr {
  enum Kind {
    UNTYPED_NODE,
  };

  Kind kind_;

  ASTErr(Kind kind) : BaseErr(), kind_(kind) {};
  ASTErr(Kind kind, std::string const &why) : BaseErr(why), kind_(kind) {};
};

struct LexerErr : BaseErr {
  enum Kind {
    UNEXP_TOKEN,
  };

  Kind m_Kind;

  LexerErr(Kind kind) : BaseErr(), m_Kind(kind) {}
  LexerErr(Kind kind, std::string const &msg) : BaseErr(msg), m_Kind(kind) {}
};

struct ParseErr : BaseErr {
  enum Kind {
    MISSING_TOKEN,
    EXPECTED_STMT,
    OUT_OF_TURN,
    OUT_OF_INIT,
    OUT_OF_LOOP,
    INVALID_OP_TOKEN,
    INVALID_LITERAL,
    INVALID_TERM,
  };

  Kind m_Kind;
  std::string m_Msg;
  ParseErr(Kind kind) : BaseErr(), m_Kind(kind) {}
  ParseErr(Kind kind, std::string const &msg) : BaseErr(msg), m_Kind(kind) {}
};

struct SymbolErr : public BaseErr {
  enum Kind {
    UNDEFINED_SYMBOL,
    REDEFINITION,
    INVALID_TYPE,
  };

  Kind m_Kind;
  SymbolErr(Kind kind) : BaseErr(), m_Kind(kind) {}
  SymbolErr(Kind kind, std::string const &msg) : BaseErr(msg), m_Kind(kind) {}
};

struct RuntimeErr : BaseErr {
  enum Kind {
    TYPE_MISMATCH,
    EMPTY_INTERP_WRAPPER,
  };

  Kind m_Kind;
  RuntimeErr(Kind kind) : BaseErr(), m_Kind(kind) {}
  RuntimeErr(Kind kind, std::string const &msg) : BaseErr(msg), m_Kind(kind) {}
};

// using InterpErr = std::variant<LexerErr, ParseErr, SymbolErr, ASTErr>;
struct InterpErr
    : std::variant<LexerErr, ParseErr, SymbolErr, ASTErr, RuntimeErr> {
  using std::variant<LexerErr, ParseErr, SymbolErr, ASTErr,
                     RuntimeErr>::variant;

  template <typename T> operator std::expected<T, InterpErr>() {
    return std::unexpected(*this);
  }
};

}; // namespace cse498
