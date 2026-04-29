#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Interpreter/errors.hpp"
#include "RobinHoodMap.hpp"
#include "agentlang.hpp"
#include "lexer.hpp"

namespace cse498 {
struct AgentWrapper;
struct SymbolTable;
};  // namespace cse498

namespace cse498::AST {
// ----------------- Nodes -------------
using namespace agentlang;
/** @brief Base `Node` type, requires inheritors to define
 * `Accept(AgentWrapper&)` and `Finalize(SymbolTable&)`.
 */
struct Node {
  emplex::Token m_Token;

  // External-facing call
  virtual std::expected<size_t, InterpErr> GetType() {
    // Nodes are by default untyped
    return ASTErr(ASTErr::UNTYPED_NODE, "Node is untyped");
  }

  /** @brief Force the derived node to call to the agent behind the given
   * `AgentWrapper` and return the resulting `Type` or error state `InterpErr`.
   */
  virtual std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) = 0;
  /** @brief Provide the derived node with a reference to the complete symbol
   * table upon the completion of parsing in order to finalize any remaining
   * missing information.
   */
  virtual std::expected<void, InterpErr> Finalize(SymbolTable&) = 0;

  Node(emplex::Token token) : m_Token(token) {};
  virtual ~Node() = 0;
};

/** @brief Base for typed nodes, derived from the base node. Intended to
 * streamline semantic analysis, but largely unused
 */
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
  }

  TypedNode(emplex::Token token) : Node(token) {};
  TypedNode(emplex::Token token, size_t type) : Node(token), m_Type(type) {};
  virtual ~TypedNode() = 0;
};

/** @brief Node representing any curly-brace-enclosed block of statements.
 */
struct StmtBlock : public Node {
  std::vector<std::unique_ptr<Node>> m_Body;

  /** @brief Add a node to the statement block. Moves the given block into the
   * vector.
   */
  void add_node(std::unique_ptr<Node>&& node) {
    m_Body.emplace_back(std::move(node));
  }

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtBlock(emplex::Token token) : Node(token), m_Body() {};
  StmtBlock(emplex::Token token, std::vector<std::unique_ptr<Node>>&& body)
      : Node(token), m_Body(std::move(body)) {}
  ~StmtBlock() = default;
};

// Filler for scaffolding
/** @brief Empty placeholder node for occurences where parsing steps should not
 * return a proper node but also should not return an error (e.g. an empty
 * statement).
 */
struct EmptyNode : public Node {
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override {
    return Types::NullType{};
  };
  std::expected<void, InterpErr> Finalize(SymbolTable&) override { return {}; };

  EmptyNode(emplex::Token token) : Node(token) {}
  ~EmptyNode() = default;
};

// -- Values --
// Literal reference
/** @brief Node representing a literal value.
 */
struct ValLiteral : public TypedNode {
  Types::Type m_Val;

  // std::expected<size_t, InterpErr> ResolveType() override;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

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
/** @brief Node representing a variable. May refer to either a user named symbol
 * or a magic (dunder) value.
 */
struct ValVariable : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Symbol;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  ValVariable(emplex::Token token, std::shared_ptr<Symbols::SymInfo> symbol)
      : TypedNode(token), m_Symbol(symbol) {}
  ~ValVariable() = default;
};

// -- Expressions --
/** @brief Node representing a unary expression.
 */
struct ExprUnary : public TypedNode {
  std::unique_ptr<Node> m_Left;

  // std::expected<size_t, InterpErr> ResolveType() override;
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  ExprUnary(emplex::Token token, std::unique_ptr<Node>&& left)
      : TypedNode(token), m_Left(std::move(left)) {};
  ~ExprUnary() = default;
};

/** @brief Node representing a binary expression (excluding assignment).
 */
struct ExprBinary : public TypedNode {
  std::unique_ptr<Node> m_Left;
  std::unique_ptr<Node> m_Right;

  // std::expected<size_t, InterpErr> ResolveType() override;
  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  ExprBinary(emplex::Token token, std::unique_ptr<Node>&& left,
             std::unique_ptr<Node>&& right)
      : TypedNode(token), m_Left(std::move(left)), m_Right(std::move(right)) {};
  ~ExprBinary() = default;
};

/** @brief Node representing the specific binary expression case of assignment.
 * Also used internally to represent assignment of function parameters before
 * evaluation.
 */
struct Assign : public TypedNode {
  std::shared_ptr<Symbols::SymInfo> m_Sym;
  std::unique_ptr<Node> m_Value;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  Assign(emplex::Token const& token, std::shared_ptr<Symbols::SymInfo> sym,
         std::unique_ptr<Node>&& value)
      : TypedNode(token), m_Sym(sym), m_Value(std::move(value)) {}
  ~Assign() = default;
};

// -- Statements --

// Agent definition
/** @brief Adent definition node.
 */
struct StmtAgentDef : public Node {
  std::unique_ptr<Node> m_Init;
  std::unique_ptr<Node> m_Turn;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtAgentDef(emplex::Token token, std::unique_ptr<Node>&& init,
               std::unique_ptr<Node>&& turn)
      : Node(token), m_Init(std::move(init)), m_Turn(std::move(turn)) {}
  ~StmtAgentDef() = default;
};

// Agent action
/** @brief Node representing an action by the agent. During evaluation, this
 * node results in a `Step` being generated. At this time, only permits
 * movement.
 */
struct StmtAction : public Node {
  // The only action is currently a movement in a direction
  // So this node MUST resolve to a direction
  std::unique_ptr<Node> m_Direction;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtAction(emplex::Token token, std::unique_ptr<Node>&& direction)
      : Node(token), m_Direction(std::move(direction)) {}
  ~StmtAction() = default;
};

// Loop (while)
/** @brief Node representing a while loop.
 */
struct StmtWhile : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_Body;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtWhile(emplex::Token token, std::unique_ptr<Node>&& condition,
            std::unique_ptr<Node>&& body)
      : Node(token),
        m_Condition(std::move(condition)),
        m_Body(std::move(body)) {}
  ~StmtWhile() = default;
};

// Loop control
/** @brief Node representing a loop control-flow statement. Only valid within a
 * loop, results in an error when either parsed outside of a loop or evaluated
 * outside of a loop.
 */
struct StmtLoopCtl : public Node {
  enum Action {
    BREAK,
    CONTINUE,
  };

  Action m_Action;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtLoopCtl(emplex::Token token, Action action)
      : Node(token), m_Action(action) {}
};

// Conditional
/** @brief Node representing a condition.
 */
struct StmtIf : public Node {
  // Must resolve to a bool-convertible type
  std::unique_ptr<Node> m_Condition;
  std::unique_ptr<Node> m_TBody;
  std::optional<std::unique_ptr<Node>> m_FBody;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtIf(emplex::Token token, std::unique_ptr<Node>&& condition,
         std::unique_ptr<Node>&& t_body, std::unique_ptr<Node>&& f_body)
      : Node(token),
        m_Condition(std::move(condition)),
        m_TBody(std::move(t_body)),
        m_FBody(std::move(f_body)) {}
  StmtIf(emplex::Token token, std::unique_ptr<Node>&& condition,
         std::unique_ptr<Node>&& t_body)
      : Node(token),
        m_Condition(std::move(condition)),
        m_TBody(std::move(t_body)),
        m_FBody({}) {}
  ~StmtIf() = default;
};

// Function return
/** @brief Node representing a return statement, only valid within a function
 * body. Results in an error both when parsed outside of a function, as well as
 * when evaluated outside of a function.
 */
struct StmtReturn : public Node {
  std::unique_ptr<Node> m_Value;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtReturn(emplex::Token token, std::unique_ptr<Node>&& val)
      : Node(token), m_Value(std::move(val)) {}
  ~StmtReturn() = default;
};

// Function
/** @brief Node representing a function body. Moved into the symbol table after
 * creation.
 */
struct StmtFunc : public Node {
  std::shared_ptr<Symbols::SymInfo> m_Symbol;
  std::unique_ptr<Node> m_Body;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  StmtFunc(emplex::Token token, std::shared_ptr<Symbols::SymInfo> symbol,
           std::unique_ptr<Node>&& body)
      : Node(token), m_Symbol(symbol), m_Body(std::move(body)) {}
  ~StmtFunc() = default;
};

// Function call
/** @brief Node representing a function call. Holds a vector of nodes
 * representing the values passed into the function called. The node shared
 * pointer `m_Body` referring to the function being called is left unset until
 * finalization.
 */
struct StmtFuncCall : public Node {
  std::shared_ptr<Symbols::SymInfo> m_Symbol;
  std::vector<std::unique_ptr<Node>> m_Args{};
  std::optional<std::shared_ptr<AST::Node>> m_Body;

  std::expected<Types::Type, InterpErr> Accept(AgentWrapper&) override;
  std::expected<void, InterpErr> Finalize(SymbolTable&) override;

  void add_node(std::unique_ptr<Node>&& n) {
    m_Args.emplace_back(std::move(n));
  }

  StmtFuncCall(emplex::Token token, std::shared_ptr<Symbols::SymInfo> sym)
      : Node(token), m_Symbol(sym), m_Body({}) {}
  ~StmtFuncCall() = default;
};

[[maybe_unused]] static std::string IDNodeForTest(Node const* node) {
  if (dynamic_cast<StmtBlock const*>(node)) {
    return "StmtBlock";
  } else if (dynamic_cast<EmptyNode const*>(node)) {
    return "EmptyNode";
  } else if (dynamic_cast<ExprUnary const*>(node)) {
    return "ExprUnary";
  } else if (dynamic_cast<ExprBinary const*>(node)) {
    return "ExprBinary";
  } else if (dynamic_cast<Assign const*>(node)) {
    return "Assign";
  } else if (dynamic_cast<StmtAgentDef const*>(node)) {
    return "StmtAgentDef";
  } else if (dynamic_cast<StmtAction const*>(node)) {
    return "StmtAction";
  } else if (dynamic_cast<StmtWhile const*>(node)) {
    return "StmtWhile";
  } else if (dynamic_cast<StmtLoopCtl const*>(node)) {
    return "StmtLoopCtl";
  } else if (dynamic_cast<StmtIf const*>(node)) {
    return "StmtIf";
  } else if (dynamic_cast<ValLiteral const*>(node)) {
    return "ValLiteral";
  } else if (dynamic_cast<ValVariable const*>(node)) {
    return "ValVariable";
  } else {
    return "Somehow none of the above";
  }
}
};  // namespace cse498::AST
