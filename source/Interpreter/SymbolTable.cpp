#include "Interpreter/SymbolTable.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <ranges>

#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"

using cse498::RobinHoodMap;
using cse498::SymbolTable;
using cse498::agentlang::Symbols::SymInfo;
using cse498::agentlang::Types::NameToType;
using SymInfoPtr = SymbolTable::SymInfoPtr;
using Scope = SymbolTable::Scope;

namespace cse498 {

/** @brief Push new symbol scope on to the stack of scopes.
 */
void SymbolTable::PushSymbolScope() { m_ScopeStack.push_back({}); }

/** @brief Remove scope from stack of scopes.
 */
void SymbolTable::PopSymbolScope() {
  assert(m_ScopeStack.size());
  m_ScopeStack.pop_back();
}

/** @brief Retrieve a symbol from the symbol table by looking up the symbol's
 * name. If the symbol cannot be found, returns an `InterpErr` containing a
 * `SymbolErr`.
 */
[[nodiscard]] std::expected<SymInfoPtr, InterpErr> SymbolTable::GetSym(
    const std::string &name) const {
  size_t idx;

  // Gives pointer to map w/ found symbol, otherwise end
  auto res = std::ranges::find_if(
      m_ScopeStack | std::views::reverse,
      [name](Scope cur_scope) { return cur_scope.contains(name); });
  if (res != m_ScopeStack.rend())
    idx = res->at(name).value();
  else
    return SymbolErr(SymbolErr::UNDEFINED_SYMBOL,
                     std::format("Could not resolve symbol {}", name));

  return m_SymbolInfo.at(idx);
}

/** @brief Retrieve a symbol from the symbol table by the symbol's index. If the
 * symbol's index is invlaid, returns an `InterpErr` containing a `SymbolErr`.
 */
[[nodiscard]] std::expected<SymInfoPtr, InterpErr> SymbolTable::GetSym(
    size_t id) const {
  if (id >= m_SymbolInfo.size())
    return SymbolErr(SymbolErr::UNDEFINED_SYMBOL,
                     std::format("Symbol index {} is invalid", id));
  return m_SymbolInfo.at(id);
}

/** @brief Add a `VarSym` symbol to the current scope with the name of the given
 * token `id_tok`'s lexeme, and the default-initialized `Type` object
 * corresponding to the given type `type_tok`.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     const Token &type_tok) {
  auto type_opt = NameToType(type_tok);
  if (!type_opt) {
    return SymbolErr(SymbolErr::INVALID_TYPE,
                     std::format("Invalid type name {}", type_tok.lexeme));
  }
  return AddSym(id_tok, *type_opt);
}
/** @brief Prepare a position to insert a new symbol into the current scope,
 * checking for erroneous redefinition. If the symbol name is valid, returns a
 * pair containing the symbol's name and the symbol's index into the vector of
 * symbols.
 */
std::expected<std::pair<std::string, size_t>, InterpErr> SymbolTable::PrepAdd(
    std::string const &name) {
  assert(m_ScopeStack.size() > 0);
  // Symbols focus on only CURRENT scope.
  auto &symbols = m_ScopeStack.back();
  if (symbols.contains(name)) {
    return SymbolErr(SymbolErr::REDEFINITION,
                     std::format("Redefinition of symbol {}", name));
  }
  size_t var_id = m_SymbolInfo.size();
  symbols[name] = var_id;

  return std::pair<std::string, size_t>{name, var_id};
}

/** @brief Overload for `PrepAdd` which takes a token.
 */
std::expected<std::pair<std::string, size_t>, InterpErr> SymbolTable::PrepAdd(
    const Token &id_tok) {
  return PrepAdd(id_tok.lexeme);
};

/** @brief Construct and add a `VarSym` symbol to the current scope, with the
 * name provided by the `id_tok` token and the value provided by `type`.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     Type type) {
  return AddSym(id_tok, VarSym(type));
}
/** @brief Directly add a `VarSym` symbol to the current scope.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     VarSym sym) {
  TRY_DECL(sym_pair, PrepAdd(id_tok));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(std::make_shared<SymInfo>(name, id_tok.line_id, sym));
  return idx;
}
/** @brief Add a magic (dunder) value to the current scope. Should only be
 * called manually, not when parsed.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(std::string id,
                                                     MagicSym sym) {
  TRY_DECL(sym_pair, PrepAdd(id));
  auto [name, idx] = sym_pair;
  m_SymbolInfo.push_back(std::make_shared<SymInfo>(name, 0, sym, sym.CanMut()));
  return m_SymbolInfo.size() - 1;
}
/** @brief Directly add function symbol to the current scope by name.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(std::string const &id,
                                                     FuncSym &&sym) {
  TRY_DECL(sym_pair, PrepAdd(id));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(
      std::make_shared<SymInfo>(name, 0, std::move(sym), m_Funcs.size()));

  m_Funcs.push_back({});

  // Functions are not assignable
  m_SymbolInfo.back()->mut = false;

  return idx;
}
/** @brief Directly add function symbol to the current scope, with the name
 * provided by the `id_tok` token.
 */
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     FuncSym &&sym) {
  TRY_DECL(sym_pair, PrepAdd(id_tok));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(std::make_shared<SymInfo>(
      name, id_tok.line_id, std::move(sym), m_Funcs.size()));

  m_Funcs.push_back({});

  // Functions are not assignable
  m_SymbolInfo.back()->mut = false;

  return idx;
}

/** @brief Take ownership of and add an AST node to the function vector,
 * returning the function's index. Functions are moved into a `shared_ptr` when
 * placed in the functions vector.
 */
std::expected<size_t, InterpErr> SymbolTable::AddFunc(
    std::unique_ptr<AST::Node> &&node) {
  m_Funcs.push_back(std::move(node));
  return m_Funcs.size() - 1;
}

/** @brief Retrieve the shared_ptr representing the function at `idx`.
 */
std::expected<std::shared_ptr<AST::Node>, InterpErr> SymbolTable::GetFunc(
    size_t idx) {
  if (idx >= m_Funcs.size())
    return SymbolErr(SymbolErr::INVALID_FUNC_IDX,
                     "Tried to retrieve function out of range");

  return m_Funcs.at(idx);
}

};  // namespace cse498
