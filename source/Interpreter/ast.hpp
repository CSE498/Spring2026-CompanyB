namespace cse498::AST {
// ----------------- Props -------------

// ----------------- Nodes -------------
// -- Expressions --
struct ExprUnary {};

struct ExprBinary {};

// -- Statements --
// Function
struct StmtFunc {};

// Variable/agent declaration
struct StmtDecl {};

// Agent definition
struct StmtAgentDef {};

// Variable definition
struct StmtVarDef {};

// Init definition
struct StmtInit {};

// Turn definition
struct StmtTurn {};

// World configuration
struct StmtCfg {};

// Agent action
struct StmtAction {};

// -- Values --
// Literal reference
struct ValLiteral {};

// Variable reference
struct ValVariable {};

// Member value reference (self or otherwise)
struct ValMember {};

}; // namespace cse498::AST
