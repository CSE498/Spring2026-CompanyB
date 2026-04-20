#include "Interpreter/Parser.hpp"
#include "Interpreter/SymbolTable.hpp"
#include "Interpreter/ast.hpp"
#include "Interpreter/errors.hpp"

#include "Interpreter/macros.hpp"
#include <variant>

namespace cse498 {

using AgentLexer::IDs;

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_func() {
  /*
  ---
  <KW_FN> <ID>([OPT <ID>:<TYPE>[, ...]]) : <TYPE> <STMT_BLOCK> <;>
  ---
  */
  if (m_InFunc)
    // I actually don't see why we couldn't, but keeping for now
    return ParseErr(ParseErr::ILLEGAL_NESTING,
                    "Cannot define a function within a function");

  TRY_DECL(func_tok, m_Lexer.UseIf(IDs::ID_KW_FN));
  TRY_DECL(func_name_tok, m_Lexer.UseIf(IDs::ID_IDENTIFIER));

  // Add function symbol so that we can resolve it if recursive calls occur
  TRY_DECL(func_sym_idx, m_Syms.AddSym(func_name_tok, FuncSym()));
  TRY_DECL(func_sym, m_Syms.GetSym(func_sym_idx));

  TRY(m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN));

  std::vector<std::shared_ptr<SymInfo>> params{};

  m_Syms.PushSymbolScope();
  while (m_Lexer.Is(IDs::ID_IDENTIFIER)) {
    // Expect: <ID><:><TYPE>[<,>]
    TRY_DECL(param_name_tok, m_Lexer.UseIf(IDs::ID_IDENTIFIER));
    TRY(m_Lexer.UseIf(IDs::ID_DELIM_CLN));
    TRY_DECL(param_type, parse_type());
    TRY_DECL(param_sym_idx, m_Syms.AddSym(param_name_tok, param_type));
    TRY_DECL(param_sym, m_Syms.GetSym(param_sym_idx));

    // Use up remaining comma if there
    if (m_Lexer.Is(IDs::ID_DELIM_COMMA))
      TRY(m_Lexer.Use());

    params.push_back(param_sym);
  }

  TRY(m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE));
  TRY(m_Lexer.UseIf(IDs::ID_DELIM_CLN));
  TRY_DECL(ret_type_tok, parse_type());

  // Now we know the signature, so we'll fill in more on the symbol
  func_sym->sym = FuncSym(params);

  m_InFunc = true;
  TRY_DECL_M(func_body, parse_stmt_block());
  m_InFunc = false;
  m_Syms.PopSymbolScope();

  // Now update the existing function sym w/ new info (params + body)
  TRY_DECL(func_body_idx, m_Syms.AddFunc(std::move(func_body)));
  func_sym->sym = FuncSym(std::move(params), func_body_idx);

  // Don't want to actually return the function
  return std::make_unique<AST::EmptyNode>(func_tok);
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_func_call() {
  /*
  ---
  <ID><(>[<EXPR>[,...]]<)>
  ---
  */
  TRY_DECL(func_id, m_Lexer.UseIf(IDs::ID_IDENTIFIER));
  TRY_DECL(func_sym, m_Syms.GetSym(func_id.lexeme));

  if (!std::holds_alternative<FuncSym>(func_sym->sym))
    return SymbolErr(
        SymbolErr::NOT_FUNCTION,
        std::format("Symbol '{}' is not a function", func_id.lexeme));

  FuncSym func_sym_data = std::get<FuncSym>(func_sym->sym);

  TRY(m_Lexer.UseIf(IDs::ID_DELIM_PAREN_OPEN));

  std::unique_ptr<AST::StmtFuncCall> func_call =
      std::make_unique<AST::StmtFuncCall>(func_id, func_sym);

  size_t arg_index = 0;
  if (!func_sym_data.m_PreloadFunc.has_value()) {
    while (!m_Lexer.Is(IDs::ID_DELIM_PAREN_CLOSE)) {
      if (arg_index >= func_sym_data.m_Params.size())
        return TempErr(TempErr::NOT_IMPLEMENTED,
                       "TODO: Put in ParseErr for too many args (vs runtime)");

      TRY_DECL_M(arg, parse_expr());

      // Arg is the value to set the params to, and we have the param symbols
      // so this is actuall y just assiugnment!!
      emplex::Token tok = arg->m_Token;
      func_call->add_node(std::make_unique<AST::Assign>(
          tok, func_sym_data.m_Params.at(arg_index), std::move(arg)));
      // func_call->add_node(std::move(arg));

      // Consume remaining comma unless next is paren close
      if (!m_Lexer.Is(IDs::ID_DELIM_PAREN_CLOSE)) {
        TRY(m_Lexer.UseIf(IDs::ID_DELIM_COMMA));
      }
    }
  } else {
    // Preloaded
    while (!m_Lexer.Is(IDs::ID_DELIM_PAREN_CLOSE)) {
      TRY_DECL_M(arg, parse_expr());

      func_call->add_node(std::move(arg));

      // Consume remaining comma unless next is paren close
      if (!m_Lexer.Is(IDs::ID_DELIM_PAREN_CLOSE)) {
        TRY(m_Lexer.UseIf(IDs::ID_DELIM_COMMA));
      }
    }
  }

  TRY(m_Lexer.UseIf(IDs::ID_DELIM_PAREN_CLOSE));

  return func_call;
}

std::expected<std::unique_ptr<AST::Node>, InterpErr> Parser::parse_return() {
  if (!m_InFunc)
    return ParseErr(ParseErr::OUT_OF_FUNC,
                    "Illegal return outside of function body");

  TRY_DECL(ret_token, m_Lexer.UseIf(IDs::ID_KW_RETURN));
  TRY_DECL_M(ret_node, parse_expr());
  return std::make_unique<AST::StmtReturn>(ret_token, std::move(ret_node));
}

}; // namespace cse498
