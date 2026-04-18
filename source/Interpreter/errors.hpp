#pragma once
#include <expected>
#include <string>
#include <utility>
#include <variant>
namespace cse498 {

struct BaseErr {
  std::string m_Why;

  BaseErr() = default;
  BaseErr(std::string const &why) : m_Why(why) {}

  template <typename T, typename E, typename Self>
  operator std::expected<T, E>(this Self &&self) {
    return std::unexpected(std::forward<Self>(self));
  }

  bool operator==(const BaseErr &) const = default;

  // template <typename Self>
  // bool operator==(this Self const &self, Self const &other) {
  //   return ((self.why_ == other.why_) && (self.m_Kind == other.m_Kind));
  // }
};

struct ASTErr : BaseErr {
  bool operator==(const ASTErr &) const = default;
  enum Kind {
    UNTYPED_NODE,
  };

  Kind m_Kind;

  ASTErr(Kind kind) : BaseErr("ASTErr"), m_Kind(kind) {};
  ASTErr(Kind kind, std::string const &why)
      : BaseErr("ASTErr: " + why), m_Kind(kind) {};
};

struct LexerErr : BaseErr {
  bool operator==(const LexerErr &) const = default;
  enum Kind {
    UNEXP_TOKEN,
  };

  Kind m_Kind;

  LexerErr(Kind kind) : BaseErr("LexerErr"), m_Kind(kind) {}
  LexerErr(Kind kind, std::string const &msg)
      : BaseErr("LexerErr: " + msg), m_Kind(kind) {}
};

struct ParseErr : BaseErr {
  bool operator==(const ParseErr &) const = default;
  enum Kind {
    MISSING_TOKEN,
    EXPECTED_STMT,
    OUT_OF_TURN,
    OUT_OF_INIT,
    OUT_OF_LOOP,
    INVALID_OP_TOKEN,
    INVALID_LITERAL,
    INVALID_TERM,
    INVALID_WORLD,
    AT_EOF,
  };

  Kind m_Kind;
  ParseErr(Kind kind) : BaseErr("ParseErr"), m_Kind(kind) {}
  ParseErr(Kind kind, std::string const &msg)
      : BaseErr("ParseErr: " + msg), m_Kind(kind) {}
};

struct SymbolErr : public BaseErr {
  bool operator==(const SymbolErr &) const = default;
  enum Kind {
    UNDEFINED_SYMBOL,
    REDEFINITION,
    INVALID_TYPE,
  };

  Kind m_Kind;
  SymbolErr(Kind kind) : BaseErr("SymbolErr"), m_Kind(kind) {}
  SymbolErr(Kind kind, std::string const &msg)
      : BaseErr("SymbolErr: " + msg), m_Kind(kind) {}
};

struct RuntimeErr : BaseErr {
  bool operator==(const RuntimeErr &) const = default;
  enum Kind {
    TYPE_MISMATCH,
    EMPTY_INTERP_WRAPPER,
    UNSUPPORTED_OP,
    NOT_BOOL_CONV,
    ENCOUNTERED_AGENT_DEF,
    INVALID_MOVE_ARG,
    TOO_MANY_MOVES
  };

  Kind m_Kind;
  RuntimeErr(Kind kind) : BaseErr("RuntimeErr"), m_Kind(kind) {}
  RuntimeErr(Kind kind, std::string const &msg)
      : BaseErr("RuntimeErr: " + msg), m_Kind(kind) {}
};

struct LoopControlErr : BaseErr {
  bool operator==(const LoopControlErr &) const = default;
  enum Kind {
    BREAK,
    CONTINUE,
  };

  Kind m_Kind;
  LoopControlErr(Kind kind) : BaseErr("LoopControlErr"), m_Kind(kind) {}
  LoopControlErr(Kind kind, std::string const &msg)
      : BaseErr("LoopControlErr: " + msg), m_Kind(kind) {}
};

// For internal errors as we implement the rest of the interpreter
struct TempErr : BaseErr {
  bool operator==(const TempErr &) const = default;
  enum Kind {
    NOT_IMPLEMENTED,
  };

  Kind m_Kind;
  TempErr(Kind kind) : BaseErr("TempErr"), m_Kind(kind) {}
  TempErr(Kind kind, std::string const &msg)
      : BaseErr("TempErr: " + msg), m_Kind(kind) {}
};

using InterpErr_T = std::variant<LexerErr, ParseErr, SymbolErr, ASTErr,
                                 RuntimeErr, LoopControlErr, TempErr>;
struct InterpErr : public InterpErr_T {
  using InterpErr_T::variant;

  template <typename T> operator std::expected<T, InterpErr>() {
    return std::unexpected(*this);
  }

  std::string ToStr() const { return std::visit(StrVis{}, *this); }

  template <typename ErrT> bool Is(ErrT::Kind err) {
    return (std::holds_alternative<ErrT>(*this) &&
            (std::get<ErrT>(*this).m_Kind == err));
  }

private:
  struct StrVis {
    std::string operator()(std::derived_from<BaseErr> auto e) {
      return e.m_Why;
    }
  };
};

}; // namespace cse498
