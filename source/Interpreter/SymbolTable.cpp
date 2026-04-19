#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/macros.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <ranges>

using cse498::RobinHoodMap;
using cse498::SymbolTable;
using cse498::agentlang::Symbols::SymInfo;
using cse498::agentlang::Types::NameToType;
using SymInfoPtr = SymbolTable::SymInfoPtr;
using Scope = SymbolTable::Scope;

namespace cse498 {

void SymbolTable::PushSymbolScope() { m_ScopeStack.push_back({}); }

void SymbolTable::PopSymbolScope() {
  assert(m_ScopeStack.size());
  m_ScopeStack.pop_back();
}

[[nodiscard]] std::expected<SymInfoPtr, InterpErr>
SymbolTable::GetSym(const std::string &name) const {
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

[[nodiscard]] std::expected<SymInfoPtr, InterpErr>
SymbolTable::GetSym(size_t id) const {
  return m_SymbolInfo.at(id);
}

[[deprecated]] std::expected<size_t, InterpErr>
SymbolTable::AddSym(const Token &id_tok, const Token &type_tok) {
  auto type_opt = NameToType(type_tok);
  if (!type_opt) {
    return SymbolErr(SymbolErr::INVALID_TYPE,
                     std::format("Invalid type name {}", type_tok.lexeme));
  }
  return AddSym(id_tok, *type_opt);
}

std::expected<std::pair<std::string, size_t>, InterpErr>
SymbolTable::PrepAdd(const Token &id_tok) {
  assert(m_ScopeStack.size() > 0);
  std::string name = id_tok.lexeme;

  // Symbols focus on only CURRENT scope.
  auto &symbols = m_ScopeStack.back();
  if (symbols.contains(name)) {
    return SymbolErr(SymbolErr::REDEFINITION,
                     std::format("Redefinition of symbol {}", name));
  }
  size_t var_id = m_SymbolInfo.size();

  return std::pair<std::string, size_t>{name, var_id};
};

std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     Type type) {
  return AddSym(id_tok, VarSym(type));
}
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     VarSym sym) {
  TRY_DECL(sym_pair, PrepAdd(id_tok));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(std::make_shared<SymInfo>(name, id_tok.line_id, sym));
  return idx;
}
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     MagicSym sym) {
  TRY_DECL(sym_pair, PrepAdd(id_tok));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(
      std::make_shared<SymInfo>(name, id_tok.line_id, sym, sym.CanMut()));
  return idx;
}
std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     FuncSym &&sym) {
  TRY_DECL(sym_pair, PrepAdd(id_tok));
  auto [name, idx] = sym_pair;

  m_SymbolInfo.push_back(
      std::make_shared<SymInfo>(name, id_tok.line_id, std::move(sym)));

  // Functions are not assignable
  m_SymbolInfo.back()->mut = false;

  return idx;
}
}; // namespace cse498
