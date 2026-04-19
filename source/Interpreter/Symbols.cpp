#include "Interpreter/agentlang.hpp"
#include "Interpreter/ast.hpp"

namespace cse498::agentlang::Symbols {

FuncSym::FuncSym(std::vector<std::shared_ptr<SymInfo>> &&params,
                 std::unique_ptr<AST::StmtBlock> &&body)
    : m_Params(std::move(params)), m_Body(std::move(body)) {}

}; // namespace cse498::agentlang::Symbols
