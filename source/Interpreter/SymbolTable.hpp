#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Interpreter/agentlang.hpp"
#include "Interpreter/errors.hpp"
#include "Interpreter/lexer.hpp"
#include "tools/RobinHoodMap.hpp"

namespace cse498 {

namespace AST {
struct Node;
};

using agentlang::Symbols::FuncSym;
using agentlang::Symbols::MagicSym;
using agentlang::Symbols::SymInfo;
using agentlang::Symbols::VarSym;
using agentlang::Types::Type;
using AgentLexer::Token;

class SymbolTable {
 public:
  using Scope = RobinHoodMap<std::string, size_t>;
  using SymInfoPtr = std::shared_ptr<SymInfo>;

 private:
  std::vector<Scope> m_ScopeStack;
  std::vector<std::shared_ptr<SymInfo>> m_SymbolInfo;
  std::vector<std::shared_ptr<AST::Node>> m_Funcs;

 public:
  void PushSymbolScope();
  void PopSymbolScope();

  // Need the actual string because we use it to index into map
  std::expected<SymInfoPtr, InterpErr> GetSym(const std::string &name) const;
  std::expected<SymInfoPtr, InterpErr> GetSym(size_t id) const;

  std::expected<size_t, InterpErr> AddSym(const Token &id_tok,
                                          const Token &type_tok);

  std::expected<std::pair<std::string, size_t>, InterpErr> PrepAdd(
      const Token &);
  std::expected<std::pair<std::string, size_t>, InterpErr> PrepAdd(
      std::string const &);

  // Magic
  std::expected<size_t, InterpErr> AddSym(std::string, MagicSym);

  // Variable
  std::expected<size_t, InterpErr> AddSym(const Token &, VarSym);
  std::expected<size_t, InterpErr> AddSym(const Token &, Type);

  // Function
  std::expected<size_t, InterpErr> AddSym(std::string const &, FuncSym &&);
  std::expected<size_t, InterpErr> AddSym(const Token &, FuncSym &&);
  std::expected<size_t, InterpErr> AddFunc(std::unique_ptr<AST::Node> &&);
  std::expected<std::shared_ptr<AST::Node>, InterpErr> GetFunc(size_t);
};

}  // namespace cse498
