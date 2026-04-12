/**
 * @file ExpressionParser.hpp
 * @brief Declaration of the ExpressionParser class.
 * @author Lewi Anamo
 * @date 2026-2-4
 */

#pragma once

#include <array>
#include <cmath>
#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cse498 {

/**
 * @enum TokenType
 * @brief Enumeration of possible token types in an expression.
 */
enum class TokenType {
  Variable,
  Literal,
  Index,
  Operator,
  Parenthesis,
  Unknown
};

/**
 * @struct Token
 * @brief Represents a token in the expression, consisting of its type and
 * value.
 */
struct Token {
  TokenType type;
  std::string value;
};

/**
 * @struct OperatorInfo
 * @brief Represents information about an operator, including its precedence,
 * associativity, and the function that applies the operator to two operands.
 * Uses a raw function pointer instead of std::function to allow constexpr
 * definition.
 */
struct OperatorInfo {
  std::string_view op;
  int precedence;
  bool rightAssociative;
  double (*apply)(double, double);
};

/**
 * @brief Compile-time array mapping operator strings to their corresponding
 * OperatorInfo. Prefered over std::unordered_map to avoid heap allocation and
 * enable constexpr evaluation.
 */
constexpr std::array<OperatorInfo, 14> OPERATORINFOMAP = {{
    {"||", -1, false,
     [](double a, double b) -> double {
       return static_cast<double>((a != 0) || (b != 0));
     }},
    {"&&", 0, false,
     [](double a, double b) -> double {
       return static_cast<double>((a != 0) && (b != 0));
     }},
    {"==", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a == b); }},
    {"!=", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a != b); }},
    {">", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a > b); }},
    {"<", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a < b); }},
    {">=", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a >= b); }},
    {"<=", 1, false,
     [](double a, double b) -> double { return static_cast<double>(a <= b); }},
    {"+", 2, false, [](double a, double b) -> double { return a + b; }},
    {"-", 2, false, [](double a, double b) -> double { return a - b; }},
    {"*", 3, false, [](double a, double b) -> double { return a * b; }},
    {"/", 3, false, [](double a, double b) -> double { return a / b; }},
    {"%", 3, false,
     [](double a, double b) -> double { return std::fmod(a, b); }},
    {"^", 4, true, [](double a, double b) -> double { return std::pow(a, b); }},
}};

/**
 * @brief Looks up an operator entry by name with a linear scan of
 * OPERATORINFOMAP (14 elements).
 * @param op The operator string to look up.
 * @return Pointer to the matching OperatorInfo, or nullptr if not found.
 */
[[nodiscard]] constexpr const OperatorInfo* findOperatorEntry(
    std::string_view op) noexcept {
  for (const auto& entry : OPERATORINFOMAP) {
    if (entry.op == op) return &entry;
  }
  return nullptr;
}

/**
 * @brief Alias representing a compiled expression that can be evaluated given
 * positional arguments and named variables.
 */
using Expression = std::function<double(const std::vector<double>&,
                                        const std::map<std::string, double>&)>;

/**
 * @class ExpressionException
 * @brief Custom exception class for handling errors related to expression
 * parsing and validation. Inherits from std::runtime_error to provide detailed
 * error messages when exceptions are thrown.
 */
class ExpressionException : public std::runtime_error {
 public:
  /**
   * @brief Constructor for ExpressionException.
   * @param msg The error message to be associated with the exception.
   */
  ExpressionException(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * @class ExpressionParser
 * @brief A class responsible for parsing and validating mathematical
 * expressions.
 */
class ExpressionParser {
 private:
  std::string expression = "";

  // Holds all the tokens that are extracted from the expression after
  // tokenization.
  std::vector<Token> tokens = {};

  std::string tokenStrings = "";

  std::set<std::string> variables = {};

  /**
   * @brief Tokenizes the raw expression string into a vector of Token objects.
   */
  void tokenize();

  /**
   * @brief Validates the tokenized expression to ensure it adheres to the
   * expected syntax and structure.
   * @return true if the expression is valid, false otherwise.
   */
  [[nodiscard]] bool validateExpr() const;

  /**
   * @brief Determines the precedence of a given operator based on
   * OPERATORINFOMAP.
   * @param op The operator for which to determine precedence.
   * @return An integer representing the precedence level of the operator.
   * Higher values indicate higher precedence.
   */
  [[nodiscard]] std::optional<int> getPrecedence(const std::string& op) const;

  /**
   * @brief Helper function to determine if a given token is an operand
   * (variable, literal, or index).
   * @param token The token to be checked.
   * @return true if the token is an operand, false otherwise.
   */
  [[nodiscard]] bool isOperand(const Token& token) const {
    return token.type == TokenType::Variable ||
           token.type == TokenType::Literal || token.type == TokenType::Index;
  }

  /**
   * @brief Helper function to determine if a given token is an operator.
   * @param token The token to be checked.
   * @return true if the token is an operator, false otherwise.
   */
  [[nodiscard]] bool isOperator(const Token& token) const {
    return token.type == TokenType::Operator;
  }

  /**
   * @brief Helper function to determine if a given token is a left parenthesis.
   * @param token The token to be checked.
   * @return true if the token is a left parenthesis, false otherwise.
   */
  [[nodiscard]] bool isLeftParen(const Token& token) const {
    return token.type == TokenType::Parenthesis && token.value == "(";
  }

  /**
   * @brief Helper function to determine if a given token is a right
   * parenthesis.
   * @param token The token to be checked.
   * @return true if the token is a right parenthesis, false otherwise.
   */
  [[nodiscard]] bool isRightParen(const Token& token) const {
    return token.type == TokenType::Parenthesis && token.value == ")";
  }

  /**
   * @brief Implements the Shunting Yard algorithm to convert the tokenized
   * expression from infix notation to postfix notation (Reverse Polish
   * Notation).
   * @param sortedTokens A reference to a vector where the sorted tokens will be
   * stored.
   * @param operatorStack A reference to a stack used for temporarily holding
   * operators during the sorting process.
   */
  void shuntingYardAlgo(std::vector<Token>& sortedTokens,
                        std::stack<Token>& operatorStack);

  /**
   * @brief Builds an evaluator function from the sorted tokens in postfix
   * notation. The returned function can be called with specific variable values
   * and index values to evaluate the expression.
   * @param sortedTokens A vector of tokens in postfix notation that represents
   * the expression to be evaluated.
   * @return An Expression that takes a vector of doubles (index values) and a
   * map of variable names to double values, and returns a double representing
   * the result of evaluating the expression with those inputs.
   */
  [[nodiscard]] static Expression buildEvaluator(
      const std::vector<Token> sortedTokens);

 public:
  /**
   * @brief Default constructor for ExpressionParser. Initializes an empty
   * expression and an empty token vector.
   */
  ExpressionParser() = default;

  /**
   * @brief Constructor for ExpressionParser that takes a raw expression string
   * as input. It initializes the expression member variable and attempts to
   * tokenize it immediately. If tokenization fails, an ExpressionException is
   * thrown with a detailed error message.
   * @param expr The raw expression string to be parsed and tokenized.
   */
  explicit ExpressionParser(const std::string& expr) : expression(expr) {
    try {
      this->tokenize();
    } catch (const ExpressionException& e) {
      throw ExpressionException("Tokenization failed: " +
                                std::string(e.what()));
    }
  }

  /**
   * @brief Retrieves the raw expression string that is currently stored in the
   * ExpressionParser instance.
   * @return A string representing the raw expression.
   */
  [[nodiscard]] std::string getRawExpression() const { return expression; }

  /**
   * @brief Sets the raw expression string and attempts to tokenize it.
   * If tokenization fails, an ExpressionException is thrown with a detailed
   * error message.
   * @param expr The new raw expression string to be set and tokenized.
   */
  void setRawExpression(const std::string& expr) {
    expression = expr;
    try {
      this->tokenize();
    } catch (const ExpressionException& e) {
      throw ExpressionException("Tokenization failed: " +
                                std::string(e.what()));
    }
  }

  /**
   * @brief Validates the expression by calling the validateExpr() method.
   * @return true if the expression is valid, false otherwise.
   */
  [[nodiscard]] bool isValid() const { return this->validateExpr(); }

  /**
   * @brief Retrieves the vector of tokens that were generated from the raw
   * expression string.
   * @return A vector of Token objects representing the tokenized expression.
   */
  [[nodiscard]] std::vector<Token> getTokens() const { return tokens; }

  /**
   * @brief Converts the vector of tokens back into a single string
   * representation of the expression. Useful for debugging and visualization
   * purposes.
   * @return A string representing the expression reconstructed from the tokens.
   */
  [[nodiscard]] std::string getTokenAsString() const { return tokenStrings; }

  /**
   * @brief Retrieves a set of unique variable names that are present in the
   * tokenized expression.
   * @return A set of strings representing the unique variable names found in
   * the expression.
   */
  [[nodiscard]] std::set<std::string> getVariables() const { return variables; }

  /**
   * @brief Parses the tokenized expression and generates a callable function
   * that can evaluate the expression given specific variable values and an
   * index vector. The returned function takes two parameters: a vector of
   * doubles representing the index values, and a map of variable names to their
   * corresponding double values. It returns a double representing the result of
   * evaluating the expression with the provided inputs.
   * @return An Expression that can be called to evaluate the expression with
   * specific variable values and index values.
   */
  [[nodiscard]] Expression parse();
};
}  // namespace cse498
