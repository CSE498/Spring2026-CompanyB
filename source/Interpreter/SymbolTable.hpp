#pragma once

#include "agentlang.hpp"
#include "lexer-gen.hpp"
#include "lexer.hpp"
#include "tools/RobinHoodMap.hpp"

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {

using agentlang::Symbols::SymInfo;
using agentlang::Types::Type;
using namespace emplex;

enum class SymbolError { UNDEFINED_SYMBOL, REDEFINITION, INVALID_TYPE };

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
  std::expected<SymInfoPtr, SymbolError> GetSym(const std::string &name) const;
  std::expected<SymInfoPtr, SymbolError> GetSym(size_t id) const;
  std::expected<size_t, SymbolError> AddSym(const Token &id_tok,
                                            const Token &type_tok);
  std::expected<size_t, SymbolError> AddSym(const Token &id_tok, Type type);
};

} // namespace cse498
