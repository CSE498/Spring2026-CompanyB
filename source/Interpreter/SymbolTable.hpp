#pragma once

#include "agentlang.hpp"
#include "tools/RobinHoodMap.hpp"

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {

using agentlang::Symbols::SymInfo;

enum class SymbolError { UNDEFINED_SYMBOL };

class SymbolTable {
public:
  using Scope = RobinHoodMap<std::string, size_t>;
  using SymInfoPtr = std::shared_ptr<SymInfo>;

private:
  std::vector<Scope> m_ScopeStack;
  std::vector<std::shared_ptr<SymInfo>> m_SymbolInfo;

public:
  void PushSymbolScope();
  void PopSymbolScope();

  // Need the actual string because we use it to index into map
  std::expected<SymInfoPtr, SymbolError> GetSym(const std::string &name);
  std::expected<SymInfoPtr, SymbolError> GetSym(size_t id);
};

} // namespace cse498
