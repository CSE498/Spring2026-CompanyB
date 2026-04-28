#pragma once
#include <array>
#include <concepts>
#include <expected>
#include <format>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace cse498 {

/** @brief The base of all `InterpErr` states. Contains the string message
 * describing the error, and defines an implicit conversion from itself to
 * `std::unexpected<T>` for the derived class.
 */
struct BaseErr {
  std::string m_Why;

  BaseErr() = default;
  BaseErr(std::string const &why) : m_Why(why) {}

  /** @brief Implicit conversion from the derived type to the `std::unexpected`
  representation of the derived type. Utilizes "deducing this".

  @tparam T The success state of the templated expected.
  @tparam E The error state of the templated expected.
  @tparam Self The type of the derived class.
  */
  template <typename T, typename E, typename Self>
  operator std::expected<T, E>(this Self && self) {
    return std::unexpected(std::forward<Self>(self));
  }

  bool operator==(const BaseErr &) const = default;
};

/** @brief Form a human-readable description of an error given the base error
 * name, the error state, and optionally a message further describing the error.
 */
static std::string Name(std::string const &base, std::string_view const &state,
                        std::string const &msg = "") {
  return std::format("{}::{}({})", base, state, msg);
}

/** @brief The `InterpErr` state representing errors which occur during the
 * construction of the AST and do not originate from a parsing failure.
 */
struct ASTErr : BaseErr {
  bool operator==(const ASTErr &) const = default;
  enum Kind {
    UNTYPED_NODE = 0,
  };

  static constexpr std::array<std::string_view, 1> m_KindNames = {
      "UNTYPED_NODE"};

  Kind m_Kind;

  ASTErr(Kind kind)
      : BaseErr(Name("ASTErr", m_KindNames[kind])), m_Kind(kind) {};
  ASTErr(Kind kind, std::string const &why)
      : BaseErr(Name("ASTErr", m_KindNames[kind], why)), m_Kind(kind) {};
};

/** @brief The `InterpErr` state representing errors which originate from the
 * lexer.
 */
struct LexerErr : BaseErr {
  bool operator==(const LexerErr &) const = default;
  enum Kind {
    UNEXP_TOKEN = 0,
  };

  static constexpr std::array<std::string_view, 1> m_KindNames = {
      "UNEXP_TOKEN"};

  Kind m_Kind;

  LexerErr(Kind kind)
      : BaseErr(Name("LexerErr", m_KindNames[kind])), m_Kind(kind) {}
  LexerErr(Kind kind, std::string const &msg)
      : BaseErr(Name("LexerErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

/** @brief The `InterpErr` state representing errors which occur during parsing
 * and originate from either an internal failure to parse or a user parsing
 * error.
 */
struct ParseErr : BaseErr {
  bool operator==(const ParseErr &) const = default;
  enum Kind {
    MISSING_TOKEN = 0,
    EXPECTED_STMT = 1,
    OUT_OF_TURN = 2,
    OUT_OF_INIT = 3,
    OUT_OF_LOOP = 4,
    INVALID_OP_TOKEN = 5,
    INVALID_LITERAL = 6,
    INVALID_TERM = 7,
    INVALID_WORLD = 8,
    AT_EOF = 9,
    ILLEGAL_ASSIGNMENT = 10,
    WORLD_MISMATCH = 11,
    MAGIC_ERR = 12,
    OUT_OF_FUNC = 13,
    ILLEGAL_NESTING = 14,
  };
  static constexpr std::array<std::string_view, 15> m_KindNames = {
      "MISSING_TOKEN",   "EXPECTED_STMT",      "OUT_OF_TURN",
      "OUT_OF_INIT",     "OUT_OF_LOOP",        "INVALID_OP_TOKEN",
      "INVALID_LITERAL", "INVALID_TERM",       "INVALID_WORLD",
      "AT_EOF",          "ILLEGAL_ASSIGNMENT", "WORLD_MISMATCH",
      "MAGIC_ERR",       "OUT_OF_FUNC",        "ILLEGAL_NESTING"};

  Kind m_Kind;
  ParseErr(Kind kind)
      : BaseErr(Name("ParseErr", m_KindNames[kind])), m_Kind(kind) {}
  ParseErr(Kind kind, std::string const &msg)
      : BaseErr(Name("ParseErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

/** @brief The `InterpErr` state representing errors which originate from
 * symbol-table failures.
 */
struct SymbolErr : public BaseErr {
  bool operator==(const SymbolErr &) const = default;
  enum Kind {
    UNDEFINED_SYMBOL = 0,
    REDEFINITION = 1,
    INVALID_TYPE = 2,
    INVALID_FUNC_IDX = 3,
    NOT_FUNCTION = 4,
  };
  static constexpr std::array<std::string_view, 5> m_KindNames = {
      "UNDEFINED_SYMBOL", "REDEFINITION", "INVALID_TYPE", "INVALID_FUNC_IDX",
      "NOT_FUNCTION"};

  Kind m_Kind;
  SymbolErr(Kind kind)
      : BaseErr(Name("SymbolErr", m_KindNames[kind])), m_Kind(kind) {}
  SymbolErr(Kind kind, std::string const &msg)
      : BaseErr(Name("SymbolErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

/** @brief The `InterpErr` state representing errors which originate during
 * runtime/evaluation.
 */
struct RuntimeErr : BaseErr {
  bool operator==(const RuntimeErr &) const = default;
  enum Kind {
    TYPE_MISMATCH = 0,
    EMPTY_INTERP_WRAPPER = 1,
    UNSUPPORTED_OP = 2,
    NOT_BOOL_CONV = 3,
    ENCOUNTERED_AGENT_DEF = 4,
    INVALID_MOVE_ARG = 5,
    TOO_MANY_MOVES = 6,
    MAGIC_ERR = 7,
    IMMUTABLE_ERR = 8,
    TOO_FEW_ARGS = 9,
    TOO_MANY_ARGS = 10,
    MISSING_RETURN = 11,
    IMPOSSIBLE_STATE = 12,
    UNRESOLVED_FUNCTION = 13,
    VALUE_ERR = 14,
    SPAWN_OUTSIDE_INIT = 15,
  };
  static constexpr std::array<std::string_view, 15> m_KindNames = {
      "TYPE_MISMATCH",
      "EMPTY_INTERP_WRAPPER",
      "UNSUPPORTED_OP",
      "NOT_BOOL_CONV",
      "ENCOUNTERED_AGENT_DEF",
      "INVALID_MOVE_ARG",
      "TOO_MANY_MOVES",
      "MAGIC_ERR",
      "IMMUTABLE_ERR",
      "TOO_FEW_ARGS",
      "TOO_MANY_ARGS",
      "MISSING_RETURN",
      "IMPOSSIBLE_STATE",
      "UNRESOLVED_FUNCTION",
      "VALUE_ERR"};

  Kind m_Kind;
  RuntimeErr(Kind kind)
      : BaseErr(Name("RuntimeErr", m_KindNames[kind])), m_Kind(kind) {}
  RuntimeErr(Kind kind, std::string const &msg)
      : BaseErr(Name("RuntimeErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

/** @brief The `InterpErr` state representing control-flow occurence. When
allowed to propagate up unhandled, these represent a failure to handle for a
control statement.

These are intended to simplify control flow during evaluation. Their presence is
not exclusively an error, only when left unhandled.
*/
struct LoopControlErr : BaseErr {
  bool operator==(const LoopControlErr &) const = default;
  enum Kind {
    BREAK = 0,
    CONTINUE = 1,
    RETURN = 2,
  };
  static constexpr std::array<std::string_view, 3> m_KindNames = {
      "BREAK", "CONTINUE", "RETURN"};

  Kind m_Kind;
  LoopControlErr(Kind kind)
      : BaseErr(Name("LoopControlErr", m_KindNames[kind])), m_Kind(kind) {}
  LoopControlErr(Kind kind, std::string const &msg)
      : BaseErr(Name("LoopControlErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

// For internal errors as we implement the rest of the interpreter
/** @brief The `InterpErr` state representing internal implementation errors
 * only.
 */
struct TempErr : BaseErr {
  bool operator==(const TempErr &) const = default;
  enum Kind {
    NOT_IMPLEMENTED = 0,
  };
  static constexpr std::array<std::string_view, 1> m_KindNames = {
      "NOT_IMPLEMENTED"};

  Kind m_Kind;
  TempErr(Kind kind)
      : BaseErr(Name("TempErr", m_KindNames[kind])), m_Kind(kind) {}
  TempErr(Kind kind, std::string const &msg)
      : BaseErr(Name("TempErr", m_KindNames[kind], msg)), m_Kind(kind) {}
};

using InterpErr_T = std::variant<LexerErr, ParseErr, SymbolErr, ASTErr,
                                 RuntimeErr, LoopControlErr, TempErr>;

/** @brief The error variant which represents any error originating from the
 * Interpreter. Defines an implicit conversion from itself to the
 * `std::unexpected` representation of itself.
 */
struct InterpErr : public InterpErr_T {
  using InterpErr_T::variant;

  /** @brief Implicit conversion from this variant to the `std::unexpected`
   * representation of this variant.
   */
  template <typename T>
  operator std::expected<T, InterpErr>() {
    return std::unexpected(*this);
  }

  /** @brief Retrieve the full message representing this error.
   */
  std::string ToStr() const { return std::visit(StrVis{}, *this); }

  /** @brief Determine whether this variant currently has the state `ErrT`.
  @tparam ErrT The error state to check that this variant currently has.
  */
  template <typename ErrT>
  bool Is(ErrT::Kind err) {
    return (std::holds_alternative<ErrT>(*this) &&
            (std::get<ErrT>(*this).m_Kind == err));
  }

 private:
  /** @brief Visitor to retrieve the string representation of the underlying
   * object.
   */
  struct StrVis {
    std::string operator()(std::derived_from<BaseErr> auto e) {
      return e.m_Why;
    }
  };
};

};  // namespace cse498
