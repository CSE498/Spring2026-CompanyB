#include "SymbolTable.hpp"
#include "agentlang.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <ranges>

using cse498::RobinHoodMap;
using cse498::SymbolError;
using cse498::SymbolTable;
using cse498::agentlang::Symbols::SymInfo;
using SymInfoPtr = SymbolTable::SymInfoPtr;
using Scope = SymbolTable::Scope;

void SymbolTable::PushSymbolScope() { m_ScopeStack.push_back({}); }

void SymbolTable::PopSymbolScope() {
  assert(m_ScopeStack.size());
  m_ScopeStack.pop_back();
}

std::expected<SymInfoPtr, SymbolError>
SymbolTable::GetSym(const std::string &name) {
  int idx;

  // Gives pointer to map w/ found symbol, otherwise end
  auto res = std::ranges::find_if(
      m_ScopeStack | std::views::reverse,
      [name](Scope cur_scope) { return cur_scope.contains(name); });
  if (res != m_ScopeStack.rend())
    idx = (*res)[name];
  else
    return std::unexpected(SymbolError::UNDEFINED_SYMBOL);

  return m_SymbolInfo[idx];
}

std::expected<SymInfoPtr, SymbolError> SymbolTable::GetSym(size_t id) {
  return m_SymbolInfo[id];
}
