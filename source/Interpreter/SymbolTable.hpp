#pragma once

#include "agentlang.hpp"
#include "tools/RobinHoodMap.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {

using agentlang::Symbols::SymInfo;

class SymbolTable {
  using Scope = RobinHoodMap<std::string, size_t>;

private:
  std::vector<Scope> m_ScopeStack;
  std::vector<std::shared_ptr<SymInfo>> m_SymbolInfo;

public:
  void PushSymbolScope();
  void PopSymbolScope();

  std::shared_ptr<SymInfo> GetSym(std::string_view name);
  std::shared_ptr<SymInfo> GetSym(size_t id);
};

} // namespace cse498
