#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"
#include "Parser.hpp"

namespace cse498 {

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_agent_def() {
  // clang-format off
  /*
  ---
  <{> <KW_INIT> <:> <STMT|STMT_BLOCK> <;> <KW_TURN> <:> <STMT|STMT_BLOCK> <;> <}> <;>
  ---
  */
  // clang-format on
  using AgentLexer::IDs;
  using AgentLexer::Token;
}

}; // namespace cse498
