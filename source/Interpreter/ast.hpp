#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "Interpreter/errors.hpp"
#include "agentlang.hpp"
#include "lexer.hpp"

namespace cse498 {
struct AgentWrapper;
};

namespace cse498::AST {
// ----------------- Nodes -------------
using namespace agentlang;
struct Node {
  emplex::Token m_Token;

  // External-facing call
  virtual std::expected<size_t, InterpErr> GetType() {
    // Nodes are by default untyped
    return ASTErr(ASTErr::UNTYPED_NODE, "Node is untyped");
  }

  // Should evaluation return nothing?
  virtual std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) = 0;

  Node(emplex::Token token) : m_Token(token) {};
  virtual ~Node() = 0;
};

struct TypedNode : public Node {
  // Cache a type !!INDEX!!
  std::optional<size_t> m_Type;

  // Internal-facing call to force the child node to resolve type
  // virtual std::expected<size_t, InterpErr> ResolveType() = 0;

  // Return the cached type if it exsits, otherwise resolve
  std::expected<size_t, InterpErr> GetType() override {
    return TempErr(TempErr::NOT_IMPLEMENTED,
                   "Not sure we're actually doing semantic analysis, so not "
                   "implementing this yet");
    // if (!m_Type.has_value()) {
    //   // Resolve the derived class' type
    //   auto type_res = ResolveType();
    //   if (!type_res.has_value())
    //     return type_res.error();

    //   // Note it internally for later calls
    //   m_Type = type_res.value();
    // }

    // // Either way now we've got it
    // return m_Type.value();
  }

  TypedNode(emplex::Token token) : Node(token) {};
  TypedNode(emplex::Token token, size_t type) : Node(token), m_Type(type) {};
  virtual ~TypedNode() = 0;
};

struct StmtBlock : public Node {
  std::vector<std::unique_ptr<Node>> m_Body;

  void add_node(std::unique_ptr<Node> &&node) {
    m_Body.emplace_back(std::move(node));
  }

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtBlock(emplex::Token token) : Node(token), m_Body() {};
  StmtBlock(emplex::Token token, std::vector<std::unique_ptr<Node>> &&body)
      : Node(token), m_Body(std::move(body)) {}
  ~StmtBlock() = default;
};

// Filler for scaffolding
struct EmptyNode : public Node {
  EmptyNode(emplex::Token token) : Node(token) {}
  ~EmptyNode() = default;
};

// -- Expressions --
struct ExprUnary : public TypedNode {
  std::unique_ptr<Node> m_Left;

  // std::expected<size_t, InterpErr> ResolveType() override;
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  ExprUnary(emplex::Token token, std::unique_ptr<Node> &&left)
      : TypedNode(token), m_Left(std::move(left)) {};
  ~ExprUnary() = default;
};

struct ExprBinary : public TypedNode {
  std::unique_ptr<Node> m_Left;
  std::unique_ptr<Node> m_Right;

  // std::expected<size_t, InterpErr> ResolveType() override;
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  ExprBinary(emplex::Token token, std::unique_ptr<Node> &&left,
             std::unique_ptr<Node> &&right)
      : TypedNode(token), m_Left(std::move(left)), m_Right(std::move(right)) {};
  ~ExprBinary() = default;
};

struct Assign : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Sym;
  std::unique_ptr<Node> m_Value;

  // std::expected<size_t, InterpErr> ResolveType() override {
  //   return m_Sym->type.index();
  // }
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  Assign(emplex::Token const &token, std::shared_ptr<Symbols::SymInfo> sym,
         std::unique_ptr<Node> &&value)
      : TypedNode(token), m_Sym(sym), m_Value(std::move(value)) {}
  ~Assign() = default;
};

// -- Statements --

// Agent definition
struct StmtAgentDef : public Node {
  std::unique_ptr<Node> m_Init;
  std::unique_ptr<Node> m_Turn;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtAgentDef(emplex::Token token, std::unique_ptr<Node> &&init,
               std::unique_ptr<Node> &&turn)
      : Node(token), m_Init(std::move(init)), m_Turn(std::move(turn)) {}
  ~StmtAgentDef() = default;
};

// Agent action
struct StmtAction : public Node {
  // The only action is currently a movement in a direction
  // So this node MUST resolve to a direction
  std::unique_ptr<Node> m_Direction;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtAction(emplex::Token token, std::unique_ptr<Node> &&direction)
      : Node(token), m_Direction(std::move(direction)) {}
  ~StmtAction() = default;
};

// Loop (while)
struct StmtWhile : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_Body;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtWhile(emplex::Token token, std::unique_ptr<Node> &&condition,
            std::unique_ptr<Node> &&body)
      : Node(token), m_Condition(std::move(condition)),
        m_Body(std::move(body)) {}
  ~StmtWhile() = default;
};

// Loop control
struct StmtLoopCtl : public Node {
  enum Action {
    BREAK,
    CONTINUE,
  };

  Action m_Action;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtLoopCtl(emplex::Token token, Action action)
      : Node(token), m_Action(action) {}
};

// Conditional
struct StmtIf : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_TBody;
  std::optional<std::unique_ptr<Node>> m_FBody;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  StmtIf(emplex::Token token, std::unique_ptr<Node> &&condition,
         std::unique_ptr<Node> &&t_body, std::unique_ptr<Node> &&f_body)
      : Node(token), m_Condition(std::move(condition)),
        m_TBody(std::move(t_body)), m_FBody(std::move(f_body)) {}
  StmtIf(emplex::Token token, std::unique_ptr<Node> &&condition,
         std::unique_ptr<Node> &&t_body)
      : Node(token), m_Condition(std::move(condition)),
        m_TBody(std::move(t_body)), m_FBody({}) {}
  ~StmtIf() = default;
};

// -- Values --
// Literal reference
struct ValLiteral : public TypedNode {
  Types::Type m_Val;

  // std::expected<size_t, InterpErr> ResolveType() override;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  // Construct from raw type value
  template <Types::TypeKind T>
  ValLiteral(emplex::Token token, T val)
      : TypedNode(token, Types::Type{std::in_place_type<T>, val}.index()),
        m_Val(Types::Type{std::in_place_type<T>, val}) {}
  // Construct from already constructed variant
  ValLiteral(emplex::Token token, Types::Type val)
      : TypedNode(token, val.index()), m_Val(val) {}
  ~ValLiteral() = default;
};

// Variable reference
struct ValVariable : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Symbol;

  // std::expected<size_t, InterpErr> ResolveType() override;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper &) override;

  ValVariable(emplex::Token token, std::shared_ptr<Symbols::SymInfo> symbol)
      : TypedNode(token, symbol->type.index()), m_Symbol(symbol) {}
  ~ValVariable() = default;
};
}; // namespace cse498::AST
