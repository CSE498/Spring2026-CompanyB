#pragma once
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/lexer.hpp"
#include <memory>
namespace cse498 {

struct ParseErr {
  enum Kind {
    MISSING_TOKEN,
    TODO,
  };

  Kind m_Kind;
  std::string m_Msg;
  ParseErr(Kind kind) : m_Kind(kind), m_Msg("") {}
  ParseErr(Kind kind, std::string const &msg) : m_Kind(kind), m_Msg(msg) {}
};

struct Parser {
  enum class Env { TRAFFIC, INFECTION } m_Env;
  SymbolTable m_Syms{};
  std::vector<std::unique_ptr<AST::Node>> m_Nodes;

  std::expected<void, ParseErr> parse(std::istream &);

private:
  cse498::AgentLexer::Lexer m_Lexer{};
  bool m_InLoop = false;
  bool m_InInit = false;
  bool m_InTurn = false;

  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_stmt();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_expr();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_stmt_block();

  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_if();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_while();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_var_def();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_agent_def();
  std::expected<std::unique_ptr<AST::Node>, ParseErr> parse_assignment();

  std::expected<Token, AgentLexer::LexerErr> parse_type();
};

}; // namespace cse498
