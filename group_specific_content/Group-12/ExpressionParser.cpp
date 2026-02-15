#include "ExpressionParser.hpp"

#include <stack>
#include <cmath>

// Used AI to come up with precedence values
int ExpressionParser::getPrecedence(const std::string& op) const {
    if (op == "^") {
        return 4;
    } else if (op == "*" || op == "/" || op == "%") {
        return 3;
    } else if (op == "+" || op == "-") {
        return 2;
    } else if (op == "<" || op == ">" || op == "<=" || op == ">="
                || op == "==" || op == "!=") {
        return 1;
    } else if (op == "&&") {
        return 0;
    } else if (op == "||") {
        return -1;
    }

    return -2;
}

std::function<double(const std::vector<double>&, const std::map<std::string, double>&)> ExpressionParser::parse() {
    try {
        this->tokenize();
    } catch (const ExpressionException& e) {
        throw ExpressionException("Tokenization failed: " + std::string(e.what()));
    }

    if (!validateExpr()) {
        throw ExpressionException("Invalid expression: " + expression);
    }

    std::vector<Token> sortedTokens = {};
    std::stack<Token> operatorStack = {};

    // Shunting Yard Algorithm
    for (const auto& token : tokens) {
        if (token.type == TokenType::Variable || token.type == TokenType::Literal || token.type == TokenType::Index) {
            sortedTokens.push_back(token);

        } else if (token.type == TokenType::Operator && token.value == "^") {
            // ^ is right associative
            operatorStack.push(token);
            
        } else if (token.type == TokenType::Operator) {
            while (!operatorStack.empty() && getPrecedence(operatorStack.top().value) >= getPrecedence(token.value)) {
                sortedTokens.push_back(operatorStack.top());
                operatorStack.pop();
            }

            operatorStack.push(token);

        } else if (token.type == TokenType::Parenthesis && token.value == ")") {
            while (!operatorStack.empty() && !(operatorStack.top().type == TokenType::Parenthesis && operatorStack.top().value == "(")) {
                sortedTokens.push_back(operatorStack.top());
                operatorStack.pop();
            }

            if (operatorStack.empty()) {
                throw ExpressionException("Mismatched parentheses in expression");
            }

            operatorStack.pop();
        } else if (token.type == TokenType::Parenthesis && token.value == "(") {
            operatorStack.push(token);
        } else {
            throw ExpressionException("Unknown token type during Parse: " + token.value);
        }
        
    }

    while (!operatorStack.empty()) {
        sortedTokens.push_back(operatorStack.top());
        operatorStack.pop();
    }

    return [sortedTokens = std::move(sortedTokens)](const std::vector<double> &args = {}, const std::map<std::string, double> &variables = {}) -> double {

        std::stack<double> evalStack = {};

        for (const auto& token: sortedTokens) {
            if (token.type == TokenType::Literal) {
                evalStack.push(std::stod(token.value));
            } else if (token.type == TokenType::Variable) {
                if (variables.find(token.value) == variables.end()) {
                    throw ExpressionException("Undefined variable: " + token.value);
                }

                evalStack.push(variables.at(token.value));
            } else if (token.type == TokenType::Index) {
                int i = std::stoi(token.value);

                if (i < 0 || i >= args.size()) {
                    throw ExpressionException("Index out of bounds: " + token.value);
                }

                evalStack.push(args.at(i));
            } else if (token.type == TokenType::Operator) {
                if (evalStack.size() < 2) {
                    throw ExpressionException("Invalid expression: Not enough operands for operator " + token.value);
                }

                double second = evalStack.top();
                evalStack.pop();

                double first = evalStack.top();
                evalStack.pop();

                // Used AI to fill in the operator cases
                if (token.value == "+") {
                    evalStack.push(first + second);
                } else if (token.value == "-") {
                    evalStack.push(first - second);
                } else if (token.value == "*") {
                    evalStack.push(first * second);
                } else if (token.value == "/") {
                    if (second == 0) {
                        throw ExpressionException("Division by zero");
                    }
                    evalStack.push(first / second);
                } else if (token.value == "^") {
                    evalStack.push(std::pow(first, second));
                } else if (token.value == "%") {
                    evalStack.push(std::fmod(first, second));
                } else if (token.value == "<") {
                    evalStack.push(first < second ? 1.0 : 0.0);
                } else if (token.value == ">") {
                    evalStack.push(first > second ? 1.0 : 0.0);
                } else if (token.value == "<=") {
                    evalStack.push(first <= second ? 1.0 : 0.0);
                } else if (token.value == ">=") {
                    evalStack.push(first >= second ? 1.0 : 0.0);
                } else if (token.value == "==") {
                    evalStack.push(first == second ? 1.0 : 0.0);
                } else if (token.value == "!=") {
                    evalStack.push(first != second ? 1.0 : 0.0);
                } else if (token.value == "&&") {
                    evalStack.push((first != 0 && second != 0) ? 1.0 : 0.0);
                } else if (token.value == "||") {
                    evalStack.push((first != 0 || second != 0) ? 1.0 : 0.0);
                } else {
                    throw ExpressionException("Unknown operator during evaluation: " + token.value);
                }
            } else {
                throw ExpressionException("Unknown token type during evaluation: " + token.value);
            }
        }

        if (evalStack.size() == 1) {
            return evalStack.top();
        } else {
            throw ExpressionException("Invalid expression");
        }
    };
}

bool ExpressionParser::validateExpr() const {
    std::vector<Token> tokens = this->getTokens();

    Token lastType = Token{TokenType::Unknown, ""};

    std::stack<Token> parenthesesStack;

    for (const auto& token: tokens) {
        if (token.type == TokenType::Variable){
            if (!std::isalpha(token.value.at(0)) && token.value.at(0) != '_') {
                return false;
            }

            // Can't follow a variable, literal, or another index
            if (lastType.type == TokenType::Variable || lastType.type == TokenType::Literal || lastType.type == TokenType::Index) {
                return false;
            }

        } else if (token.type == TokenType::Literal) {
            try {
                std::stod(token.value);
            } catch (const std::exception& e) {
                return false;
            }

            // Can't follow a variable, literal, or another index
            if (lastType.type == TokenType::Variable || lastType.type == TokenType::Literal || lastType.type == TokenType::Index) {
                return false;
            }

        } else if (token.type == TokenType::Index) {
            try {
                std::stoi(token.value);
            } catch (const std::exception& e) {
                return false;
            }

            // Can't follow a variable, literal, or another index
            if (lastType.type == TokenType::Variable || lastType.type == TokenType::Literal || lastType.type == TokenType::Index) {
                return false;
            }

        } else if (token.type == TokenType::Operator) {

            if (lastType.type == TokenType::Operator || (lastType.type == TokenType::Parenthesis && lastType.value == "(") || lastType.type == TokenType::Unknown) {
                // Can't start with an operator or follow and operator
                return false;
            }

        } else if (token.type == TokenType::Parenthesis) {
            if (token.value == "(") {
                if (lastType.type == TokenType::Variable || lastType.type == TokenType::Literal || lastType.type == TokenType::Index || (lastType.type == TokenType::Parenthesis && lastType.value == ")")) {
                    // Can't follow a variable, literal, or index
                    return false;
                }
                parenthesesStack.push(token);

            } else if (token.value == ")") {
                if (lastType.type == TokenType::Operator || (lastType.type == TokenType::Parenthesis && lastType.value == "(") ||lastType.type == TokenType::Unknown) {
                    // Can't follow an operator or another parenthesis
                    return false;
                }

                // Check parentheses balance
                if (parenthesesStack.empty()) {
                    return false;
                } else {
                    Token prev_paren = parenthesesStack.top();

                    if (prev_paren.value == "(") {
                        parenthesesStack.pop();
                    } else {
                        return false;
                    }
                }
            }

        } else {
            return false;

        }

        lastType = token;
    }

    // Expr can't be empty, end with an operator, or have unbalanced parentheses
    if (lastType.type == TokenType::Operator || lastType.type == TokenType::Unknown || !parenthesesStack.empty()) {
        return false;
    }

    return true;
}

void ExpressionParser::tokenize() {
    tokens.clear();

    int i = 0;

    while (i < expression.size()) {
        char c = expression[i];

        if (std::isalpha(c)) {
            std::string var = "";
            var += c;

            while (i + 1 < expression.size() && (std::isalnum(expression.at(i + 1)) || expression.at(i + 1) == '_')) {
                var += expression.at(++i);
            }

            Token token{TokenType::Variable, var};
            tokens.push_back(token);

        } else if (std::isdigit(c) || c == '.'){
            std::string literal = "";
            literal += c;

            bool hasDecimal = false;
            bool hasDigit = false;

            // Check if there is at least one digit and at most one decimal point in the number
            if (std::isdigit(c)) {
                hasDigit = true;
            } else if (c == '.') {
                hasDecimal = true;
            }

            while (i + 1 < expression.size() && (std::isdigit(expression.at(i + 1)) || expression.at(i + 1) == '.')) {
                if (std::isdigit(expression.at(i + 1)) && !hasDigit) {
                    hasDigit = true;
                } else if (expression.at(i + 1) == '.') {
                    if (hasDecimal) {
                        throw ExpressionException("Invalid literal: Multiple decimal points in a single number");
                    }
                    
                    hasDecimal = true;
                }

                literal += expression.at(++i);
            }

            if (!hasDigit) {
                throw ExpressionException("Invalid literal: No digits found in a number");
            }

            Token token{TokenType::Literal, literal};
            tokens.push_back(token);
            
        } else if (c == '{') {
            std::string index = "";

            while (i + 1 < expression.size()) {
                if (expression.at(i + 1) == '}') {
                    ++i;
                    break;
                } else if (expression.at(i + 1) == '{') {
                    throw ExpressionException("Invalid index: Nested braces are not allowed");
                } else if (!std::isdigit(expression.at(i + 1))) {
                    throw ExpressionException("Invalid index: Non-numeric characters in index");
                }
                
                index += expression.at(++i);
            }
            
            if (expression.at(i) != '}') {
                throw ExpressionException("Invalid index: Missing closing brace '}'");
            }

            if (index.empty()) {
                throw ExpressionException("Invalid index: Empty index");
            }

            Token token{TokenType::Index, index};
            tokens.push_back(token);

        } else if (c == '(' || c == ')') {
            Token token{TokenType::Parenthesis, std::string(1, c)};
            tokens.push_back(token);
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || 
                    c == '^' || c == '%') {
            Token token{TokenType::Operator, std::string(1, c)};
            tokens.push_back(token);
        } else if (c == '<' || c == '>') {
            if (i + 1 < expression.size() && expression.at(i + 1) == '=') {
                Token token{TokenType::Operator, std::string(1, c) + "="};
                tokens.push_back(token);
                ++i;
            } else {
                Token token{TokenType::Operator, std::string(1, c)};
                tokens.push_back(token);
            }

        } else if (c == '=' || c == '&' || c == '|') {
            if (i + 1 < expression.size() && expression.at(i + 1) == c) {
                tokens.push_back(Token{TokenType::Operator, std::string(2, c)});
                ++i;
            } else {
                throw ExpressionException(std::string("Invalid operator: Single '") + c + "' not supported");
            }

        } else if (c == '!') {
            if (i + 1 < expression.size() && expression.at(i + 1) == '=') {
                Token token{TokenType::Operator, "!="};
                tokens.push_back(token);
                ++i;
            } else {
                throw ExpressionException("Invalid operator: Single '!' not supported");
            }

        } else {
            throw ExpressionException(std::string("Invalid character in expression: '") + c + "'");
        }

        ++i;
    }

    return;
}
