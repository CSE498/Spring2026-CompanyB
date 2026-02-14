/**
 * @file ExpressionParser.hpp
 * @brief Declaration of the ExpressionParser class.
 * @author Lewi Anamo
 * @date 2026-2-4
 */

 #pragma once

#include <string>
#include <vector>
#include <functional>

enum class TokenType {
    Variable,
    Literal,
    Index,
    Operator,
    Parenthesis
};

struct Token {
    TokenType type;
    std::string value;
};

class ExpressionParser {
private:
    std::string expression = "";
    std::vector<Token> tokens = {};

public:
    ExpressionParser() = default;
    explicit ExpressionParser(const std::string& expr) : expression(expr) {
        tokenize();
    }

    std::string getExpression() const {
        return expression;
    }

    void setExpression(const std::string& expr) {
        expression = expr;
        tokenize();
    }

    std::vector<Token> getTokens() const {
            return tokens;
        }
    };
    
    void tokenize();
    