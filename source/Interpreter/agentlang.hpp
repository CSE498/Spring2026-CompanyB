#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>

#include "Interpreter/errors.hpp"
#include "RobinHoodMap.hpp"
#include "core.hpp"
#include "core/WorldPosition.hpp"
#include "lexer.hpp"

namespace cse498 {

// Forward declarations
namespace AST {
struct Node;
struct StmtBlock;
struct ValVariable;
};  // namespace AST

namespace agentlang::Types {

// Alias so that the internal "point" and the type "point" match
using PointTy = WorldPosition;
using str = std::string;

enum class Dir {
  LEFT,
  RIGHT,
  UP,
  DOWN,
};

// TODO : How are agents represented internally?
struct Car {};
struct Student {};
struct NullType {};
using Type =
    std::variant<bool, int, double, str, PointTy, Dir, Car, Student, NullType>;

// Concept satisfied if T is a valid agentlang type
/** @brief Named constraint requiring that the templated type `T` is a valid
 * agentlang type.
 * @tparam T The type to verify as a legal agentlang type
 */
template <typename T>
concept TypeKind = Concepts::IsOneOf<T, bool, int, double, str, PointTy, Dir,
                                     Car, Student, NullType>;
static const std::array<std::string_view, 9> TYPE_NAMES = {
    "bool",      "int", "double",  "str", "point",
    "direction", "car", "student", "null"};

/** @brief Retrieve the agentlang keyword representation of the templated type
 * `TypeKind`
 * @tparam T The type to retrieve the agentlang keyword for
 */
template <TypeKind T>
inline std::string_view TypeToName() {
  return TYPE_NAMES.at(StaticUtil::variant_index<Type, T>());
}

/** @brief Retrieve the agentlang keyword representation of the input `Type`
 * variant.
 * @param t Type to retrieve the agentlang keyword representation of.
 */
inline std::string_view TypeVariantToName(const Type& t) {
  return std::visit(
      [](auto&& v) { return TypeToName<std::decay_t<decltype(v)>>(); }, t);
}

/** @brief Retrieve a default-constructed instance of the `Type` object
 * corresponding to the given token, if it exists. Otherwise, return an empty
 * `Optional`.
 * @param type_tok The token containing the desired type.
 */
inline std::optional<Type> NameToType(const emplex::Token& type_tok) {
  static const std::unordered_map<std::string, Type> type_map = {
      {"bool", bool{}}, {"int", int{}},         {"double", double{}},
      {"str", str{}},   {"point", PointTy{}},   {"direction", Dir{}},
      {"car", Car{}},   {"student", Student{}},
  };

  auto it = type_map.find(type_tok.lexeme);
  if (it != type_map.end()) {
    return it->second;
  }
  return std::nullopt;
}
};  // namespace agentlang::Types

namespace agentlang::Operators {

constexpr int MAX_PREC = 7;

struct OpInfo {
  enum class Assoc {
    LEFT,
    RIGHT,
    NEITHER,
  };

  int m_Prec;
  Assoc m_Assoc;

  /*
  Precedences:
   - 1 : member access (self.X)
   - 2 : Unary ops (lnot, minus)
   - 3 : Exp
   - 4 : Mult, div, rem
   - 5 : Add, sub
   - 6 : Comparisons (all)
   - 7 : Assignment
  */

  /** @brief Retrieve an `OpInfo` corresponding to the given operator token
   *   `token`. If the token is a valid binary operator, the `OpInfo` found is
   *   returned. Otherwise, an `InterpErr` is returned describing the error
   * which occured.
   *   @param token The token containing the desired operator.
   */
  static std::expected<OpInfo, InterpErr> FromBinary(
      AgentLexer::Token const& token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
      case IDs::ID_OP_MULT:
      case IDs::ID_OP_DIVIDE:
      case IDs::ID_OP_REM:
        return OpInfo{4, Assoc::LEFT};
      case IDs::ID_OP_MINUS:
      case IDs::ID_OP_ADD:
        return OpInfo{5, Assoc::LEFT};
      case IDs::ID_CMP_EQ:
      case IDs::ID_CMP_NEQ:
      case IDs::ID_CMP_LT:
      case IDs::ID_CMP_LEQ:
      case IDs::ID_CMP_GT:
      case IDs::ID_CMP_GEQ:
        return OpInfo{6, Assoc::NEITHER};
      case IDs::ID_OP_ASSIGN:
        return OpInfo{7, Assoc::RIGHT};
      default:
        return std::unexpected(ParseErr(
            ParseErr::INVALID_OP_TOKEN,
            std::format("Given token '{}' is not a valid binary operator",
                        AgentLexer::TokenName(token.id))));
    };
  }
  /** @brief Retrieve an `OpInfo` corresponding to the given operator token
   *   `token`. If the token is a valid unary operator, the `OpInfo` found is
   *   returned. Otherwise, an `InterpErr` is returned describing the error
   * which occured.
   *   @param token The token containing the desired operator.
   */
  static std::expected<OpInfo, InterpErr> FromUnary(
      AgentLexer::Token const& token) {
    using AgentLexer::IDs;
    // Use fallthrough to evaluate as logical-or
    switch (token.id) {
      case IDs::ID_OP_MINUS:
      case IDs::ID_OP_LNOT:
        return OpInfo{2, Assoc::RIGHT};
      default:
        return std::unexpected(ParseErr(
            ParseErr::INVALID_OP_TOKEN,
            std::format("Given token '{}' is not a valid unary operator",
                        AgentLexer::TokenName(token.id))));
    };
  }

  // The tokens are ints and the operators are tightly grouped
  // so we can just check a range
  static constexpr int MIN_OP = AgentLexer::IDs::ID_OP_ADD;
  static constexpr int MAX_OP = AgentLexer::IDs::ID_OP_MINUS;

  /** @brief Determine whether the given token represents a valid operator,
   *   either unary or binary.
   *   @param token The token to check.
   */
  static bool IsOpToken(AgentLexer::Token const& token) {
    return ((token.id <= MAX_OP) && (token.id >= MIN_OP));
  }
};
};  // namespace agentlang::Operators

namespace agentlang::Symbols {

using agentlang::Types::Type;

// Forwrad decl.
struct SymInfo;

/** @brief The symbol variant representing any user-defined variable. Contains
 * only a `Type` variant representing the current value of that symbol.
 */
struct VarSym {
  Type m_Type;

  VarSym(Type t) : m_Type(t) {}
};

/** @brief The symbol variant representing any function, user-defined or
 * preloaded. Contains a vector of `shared_ptr`s to the function's parameters,
 * the index of the function's body in the symbol table's function vector, and
 * an optional containing a preloaded function. This field is left in the
 * optional's empty state for any user-defined functions. For preloaded
 * functions, this field contains a `std::function` object which holds the
 * preloaded function.
 */
struct FuncSym {
  std::vector<std::shared_ptr<SymInfo>> m_Params;
  size_t m_BodyIdx;
  std::optional<
      std::function<std::expected<Type, InterpErr>(std::vector<Type>&&)>>
      m_PreloadFunc = {};

  FuncSym() : m_Params({}) {}
  FuncSym(std::vector<std::shared_ptr<SymInfo>> params)
      : m_Params(std::move(params)) {};
  FuncSym(std::vector<std::shared_ptr<SymInfo>>&& params, size_t idx)
      : m_Params(std::move(params)), m_BodyIdx(idx) {};

  template <typename Func>
    requires std::is_convertible_v<Func,
                                   std::function<std::expected<Type, InterpErr>(
                                       std::vector<Type>&&)>>
  FuncSym(Func f) : m_Params({}), m_BodyIdx(0), m_PreloadFunc(f) {}
};

/** @brief The symbol variant representing preloaded "magic" (or dunder) values.
 * Contains only an enum representing the desired magic value.
 */
struct MagicSym {
  enum class Value {
    SPAWN,        // __spawn__, valid both
    POSITION,     // __position__, valid both
    DESTINATION,  // __destination__, valid both
    INFECTED,     // __infected__, valid infection
    SUSCEPTIBLE,  // __susceptible__, valid infection
    RECOVERED,    // __recovered__, valid infection
    FACING,       // __facing__, valid traffic
  };

  Value m_Value;

  /** @brief Determine whether the currently held `Value` state can be
   *   considered mutable.
   *
   *   Current mutable values are:
   *     - DESTINATION
   *     - SPAWN
   */
  bool CanMut() {
    return (m_Value == Value::DESTINATION || m_Value == Value::SPAWN);
  };

  /** @brief Convert the given magic value state into its string representation
   *   without the surrounding underscores.
   *
   *   @param m The value to retrieve the state of.
   */
  static std::string ToStr(Value m) {
    switch (m) {
      case Value::POSITION:
        return "position";
      case Value::DESTINATION:
        return "destination";
      case Value::INFECTED:
        return "infected";
      case Value::SUSCEPTIBLE:
        return "susceptible";
      case Value::RECOVERED:
        return "recovered";
      case Value::FACING:
        return "facing";
      case Value::SPAWN:
        return "spawn";
    };
    return "ERROR";
  }

  MagicSym(Value value) : m_Value(value) {}
};

using SymVariant_T = std::variant<VarSym, FuncSym, MagicSym>;

/** @brief The variant representing the concrete information about symbol in the
 * symbol table. Derived from the variant of the symbol types.
 *
 * Permits most usage a normal variant would, namely
 * `std::holds_alternative<T>(V)` and `std::get<T>(V)`. Two symbol variants are
 * considered equal if they both hold the same state and their underlying
 * objects are equal.
 */
struct SymVariant : SymVariant_T {
  using SymVariant_T::variant;
  using SymVariant_T::operator=;

  /** @brief Retrieve a string describing the current state of this symbol
   * variant.
   */
  std::string StateAsStr() const {
    switch (this->index()) {
      case 0:
        return "variant";
      case 1:
        return "function";
      case 2:
        return "magic";
      default:
        return "invalid_sym";
    }
  }

  /** @brief Determine whether this symbol variant currently holds the state
   *   `T`.
   *   @tparam T The state to check whether this symbol variant holds.
   */
  template <typename T>
  bool IsA() const {
    return this->index() == StaticUtil::variant_index<SymVariant_T, T>();
  }

  /** @brief Apply `std::get<T>(V)` to this symbol variant, returning the
   *   object. Currently does no checking that the underlying object contains
   * the correct state.
   *   @tparam The state to retrieve the underlying object representation of.
   */
  template <typename T>
  T As() const {
    return std::get<T>(*this);
  }
};

/** @brief The complete information representing a symbol in the symbol table.
 * Only the `sym` field is under the symbol variant, all other fields are
 * consistent across types.
 */
struct SymInfo {
  std::string name;
  size_t line_def;
  SymVariant sym;
  bool mut = true;

  template <typename T>
  SymInfo(std::string _name, size_t _line_def, T&& _sym)
      : name(_name), line_def(_line_def), sym(std::forward<T>(_sym)) {}

  template <typename T>
  SymInfo(std::string _name, size_t _line_def, T&& _sym, bool _mut)
      : name(_name),
        line_def(_line_def),
        sym(std::forward<T>(_sym)),
        mut(_mut) {}
};

}  // namespace agentlang::Symbols

};  // namespace cse498
