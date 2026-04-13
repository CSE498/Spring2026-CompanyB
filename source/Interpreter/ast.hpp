#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "agentlang.hpp"
#include "lexer.hpp"

namespace cse498::AST {
// ----------------- Nodes -------------
using namespace agentlang;
struct Node {
  emplex::Token m_Token;

  // External-facing call
  virtual std::expected<size_t, InterpErr> GetType() {
    // Nodes are by default untyped
    return std::unexpected(ASTErr(ASTErr::UNTYPED_NODE, "Node is untyped"));
  }

  Node(emplex::Token token) : m_Token(token) {};
  virtual ~Node() = 0;
};

struct TypedNode : public Node {
  // Cache a type !!INDEX!!
  std::optional<size_t> m_Type;

  // Internal-facing call to force the child node to resolve type
  virtual std::expected<size_t, InterpErr> ResolveType() = 0;

  // Return the cached type if it exsits, otherwise resolve
  std::expected<size_t, InterpErr> GetType() override {
    if (!m_Type.has_value())
      return ResolveType();
    else
      return m_Type.value();
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
  std::unique_ptr<Operators::Operator> m_Op;
  std::unique_ptr<Node> m_Left;

  ExprUnary(emplex::Token token, std::unique_ptr<Node> &&left,
            std::unique_ptr<Operators::Operator> &&op)
      : TypedNode(token), m_Op(std::move(op)), m_Left(std::move(left)) {};
  ~ExprUnary() = default;
};

struct ExprBinary : public TypedNode {
  std::unique_ptr<Operators::Operator> m_Op;
  std::unique_ptr<Node> m_Left;
  std::unique_ptr<Node> m_Right;

  ExprBinary(emplex::Token token, std::unique_ptr<Node> &&left,
             std::unique_ptr<Node> &&right,
             std::unique_ptr<Operators::Operator> &&op)
      : TypedNode(token), m_Op(std::move(op)), m_Left(std::move(left)),
        m_Right(std::move(right)) {};
  ~ExprBinary() = default;
};

struct Assign : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Sym;
  std::unique_ptr<Node> m_Value;

  std::expected<size_t, InterpErr> ResolveType() { return m_Sym->type.index(); }

  Assign(emplex::Token const &token, std::shared_ptr<Symbols::SymInfo> sym,
         std::unique_ptr<Node> &&value)
      : TypedNode(token), m_Sym(sym), m_Value(std::move(value)) {}
  ~Assign() = default;
};

// -- Statements --
// Function
struct StmtFunc : public TypedNode {
  // TODO : Not including in alpha
  /*
  std::unique_ptr<StmtBlock> m_Body;
  std::vector<size_t> m_SignatureTypes;

  StmtFunc(emplex::Token token, std::unique_ptr<StmtBlock> &&body,
           std::vector<size_t> &&signature_types, Types::Type return_t)
      : TypedNode(token, return_t), m_Body(std::move(body)),
        m_SignatureTypes(std::move(signature_types)) {};
  */
};

// Agent definition
struct StmtAgentDef : public Node {
  std::unique_ptr<Node> m_Init;
  std::unique_ptr<Node> m_Turn;
  std::shared_ptr<Symbols::SymInfo> m_Sym;

  StmtAgentDef(emplex::Token token, std::unique_ptr<Node> &&init,
               std::unique_ptr<Node> &&turn,
               std::shared_ptr<Symbols::SymInfo> sym)
      : Node(token), m_Init(std::move(init)), m_Turn(std::move(turn)),
        m_Sym(sym) {}
  ~StmtAgentDef() = default;
};

// Agent action
struct StmtAction : public Node {
  // The only action is currently a movement in a direction
  // So this node MUST resolve to a direction
  std::unique_ptr<Node> m_Direction;

  StmtAction(emplex::Token token, std::unique_ptr<Node> &&direction)
      : Node(token), m_Direction(std::move(direction)) {}
  ~StmtAction() = default;
};

// Loop (while)
struct StmtWhile : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_Body;

  StmtWhile(emplex::Token token, std::unique_ptr<Node> &&condition,
            std::unique_ptr<Node> &&body)
      : Node(token), m_Condition(std::move(condition)),
        m_Body(std::move(body)) {}
  ~StmtWhile() = default;
};

// Conditional
struct StmtIf : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_TBody;
  std::optional<std::unique_ptr<Node>> m_FBody;

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

  // Construct from raw type value
  template <Types::TypeKind T>
  ValLiteral(emplex::Token token, T val)
      : TypedNode(token, StaticUtil::variant_index<Types::Type, T>()),
        m_Val(Types::Type{std::in_place_type<T>, val}) {}
  // Construct from already constructed variant
  ValLiteral(emplex::Token token, Types::Type val)
      : TypedNode(token, val.index()), m_Val(val) {}
  ~ValLiteral() = default;
};

// Variable reference
struct ValVariable : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Symbol;

  ValVariable(emplex::Token token, std::shared_ptr<Symbols::SymInfo> symbol)
      : TypedNode(token, symbol->type.index()), m_Symbol(symbol) {}
  ~ValVariable() = default;
};

}; // namespace cse498::AST
