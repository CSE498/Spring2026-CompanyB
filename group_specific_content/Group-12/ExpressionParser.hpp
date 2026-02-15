/**
 * @file ExpressionParser.hpp
 * @brief Declaration of the ExpressionParser class.
 * @author Lewi Anamo
 * @date 2026-2-4
 */

 #pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>
#include <set>

enum class TokenType {
    Variable,
    Literal,
    Index,
    Operator,
    Parenthesis,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
};

class ExpressionParser {
private:
    std::string expression = "";
    std::vector<Token> tokens = {};

    void tokenize();
    bool validateExpr() const;
    int getPrecedence(const std::string& op) const;
public:
    ExpressionParser() = default;
    explicit ExpressionParser(const std::string& expr) : expression(expr) {}

    std::string getRawExpression() const {
        return expression;
    }

    void setRawExpression(const std::string& expr) {
        expression = expr;
    }

    std::vector<Token> getTokens() const {
            return tokens;
    }

    std::string getTokenAsString() const {
        std::string result = "";
        for (const auto& token : tokens) {
            result += token.value + " ";
        }
        return result;
    }

    std::set<std::string> getVariables() const {
        std::set<std::string> result;
        for (const auto& token : tokens) {
            if (token.type == TokenType::Variable) {
                result.insert(token.value);
            }
        }
        return result;
    }

    std::function<double(const std::vector<double>&, const std::map<std::string, double>&)> parse();
};


class ExpressionException : public std::runtime_error {
public:
    ExpressionException(const std::string& msg) : std::runtime_error(msg) {}
};