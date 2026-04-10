#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "agentlang.hpp"
#include "lexer.hpp"

namespace cse498::AST {
// ----------------- Utility -------------
struct ASTErr {
  enum Kind {
    UNTYPED_NODE,
  };

  Kind kind_;
  std::string why_;

  ASTErr(Kind kind) : kind_(kind) {};
  ASTErr(Kind kind, std::string const &why) : kind_(kind), why_(why) {};
};

// ----------------- Nodes -------------
using namespace agentlang;
struct Node {
  emplex::Token token_;

  // External-facing call
  virtual std::expected<Types::Type, ASTErr> get_type() {
    // Nodes are by default untyped
    return std::unexpected(ASTErr(ASTErr::UNTYPED_NODE, "Node is untyped"));
  }

  Node(emplex::Token token) : token_(token) {};
  virtual ~Node() = 0;
};

struct TypedNode : public Node {
  // Cache a type
  std::optional<Types::Type> type_;

  // Internal-facing call to force the child node to resolve type
  virtual std::expected<Types::Type, ASTErr> resolve_type() = 0;

  // Return the cached type if it exsits, otherwise resolve
  std::expected<Types::Type, ASTErr> get_type() override {
    if (!type_.has_value())
      return resolve_type();
    else
      return type_.value();
  }

  TypedNode(emplex::Token token) : Node(token) {};
  TypedNode(emplex::Token token, Types::Type type)
      : Node(token), type_(type) {};
  virtual ~TypedNode() = 0;
};

// -- Expressions --
struct ExprUnary : public TypedNode {
  std::unique_ptr<Operators::Operator> op_;
  std::unique_ptr<Node> left_;

  ExprUnary(emplex::Token token, std::unique_ptr<Node> &&left,
            std::unique_ptr<Operators::Operator> &&op)
      : TypedNode(token), op_(std::move(op)), left_(std::move(left)) {};
};

struct ExprBinary : public TypedNode {
  std::unique_ptr<Operators::Operator> op_;
  std::unique_ptr<Node> left_;
  std::unique_ptr<Node> right_;

  ExprBinary(emplex::Token token, std::unique_ptr<Node> &&left,
             std::unique_ptr<Node> &&right,
             std::unique_ptr<Operators::Operator> &&op)
      : TypedNode(token), op_(std::move(op)), left_(std::move(left)),
        right_(std::move(right)) {};
};

// -- Statements --
// Function
struct StmtFunc : public TypedNode {
  // TODO - Waiting on a symbol type
};

// Variable definition
struct StmtVarDef : public Node {};

// Init definition
struct StmtInit : public Node {};

// Turn definition
struct StmtTurn : public Node {};

// Agent definition
struct StmtAgentDef : public Node {
  std::unique_ptr<StmtInit> init_;
  std::unique_ptr<StmtTurn> turn_;
};

// Agent action
struct StmtAction : public Node {};

// -- Values --
// Literal reference
struct ValLiteral : public Node {
  Types::Type val_;
};

// Variable reference
struct ValVariable : public Node {};

// Member value reference (self or otherwise)
struct ValMember : public Node {};

}; // namespace cse498::AST
