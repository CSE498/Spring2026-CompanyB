#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"

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
    std::format("Could not resolve symbol {}",
    name)
    );

  return m_SymbolInfo.at(idx);
}

[[nodiscard]] std::expected<SymInfoPtr, InterpErr>
SymbolTable::GetSym(size_t id) const {
  return m_SymbolInfo.at(id);
}

std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     const Token &type_tok) {
  auto type_opt = NameToType(type_tok);
  if (!type_opt) {
    return SymbolErr(SymbolErr::INVALID_TYPE, 
    std::format("Invalid type name {}",
    type_tok.lexeme));
  }
  return AddSym(id_tok, *type_opt);
}

std::expected<size_t, InterpErr> SymbolTable::AddSym(const Token &id_tok,
                                                     Type type) {
  assert(m_ScopeStack.size() > 0);
  std::string name = id_tok.lexeme;

  // Symbols focus on only CURRENT scope.
  auto &symbols = m_ScopeStack.back();
  if (symbols.contains(name)) {
    return SymbolErr(SymbolErr::REDEFINITION,
    std::format("Redefinition of symbol {}",
    name));
  }
  size_t var_id = m_SymbolInfo.size();
  m_SymbolInfo.push_back(std::make_shared<SymInfo>(name, id_tok.line_id, type));
  symbols[name] = var_id;

  return var_id;
}
}; // namespace cse498
