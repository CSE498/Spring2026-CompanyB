#include "ExpressionParser.hpp"

namespace cse498 {

    std::optional<int> ExpressionParser::getPrecedence(const std::string& op) const {
        // Return nullopt for unknown operators, otherwise get what is store in OPERATORINFOMAP
        if (OPERATORINFOMAP.find(op) == OPERATORINFOMAP.end()) {
            return std::nullopt;
        } else {
            return OPERATORINFOMAP.at(op).precedence;
        }
    }

    void ExpressionParser::shantingYardAlgo(std::vector<Token>& sortedTokens, std::stack<Token>& operatorStack) {
        
        // Shunting Yard Algorithm
        for (const auto& token : tokens) {
            if (isOperand(token)) {
                sortedTokens.push_back(token);

            } else if (isOperator(token) && token.value == "^") {
                // ^ is right associative
                operatorStack.push(token);
                
            } else if (isOperator(token)) {
                while (!operatorStack.empty() && getPrecedence(operatorStack.top().value).has_value() 
                        && getPrecedence(operatorStack.top().value).value() >= getPrecedence(token.value).value()) {
                    sortedTokens.push_back(operatorStack.top());
                    operatorStack.pop();
                }

                operatorStack.push(token);

            } else if (isRightParen(token)) {
                while (!operatorStack.empty() && !(isLeftParen(operatorStack.top()))) {
                    sortedTokens.push_back(operatorStack.top());
                    operatorStack.pop();
                }

                if (operatorStack.empty()) {
                    throw ExpressionException("Mismatched parentheses in expression");
                }

                operatorStack.pop();
            } else if (isLeftParen(token)) {
                operatorStack.push(token);
            } else {
                throw ExpressionException("Unknown token type during Parse: " + token.value);
            }
            
        }
    };

    std::function<double(const std::vector<double>&, const std::map<std::string, double>&)> ExpressionParser::buildEvaluator(const std::vector<Token> sortedTokens) {
        // Capture the sorted tokens by value and return a lambda function that takes in the arguments and variables for evaluation
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

                    if (i < 0) {
                        throw ExpressionException("Invalid index: Negative index " + token.value);
                    }

                    std::size_t idx = static_cast<std::size_t>(i);

                    if (idx >= args.size()) {
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

                    if (token.value == "/" && second == 0) {
                        throw ExpressionException("Division by zero");
                    }

                    if (OPERATORINFOMAP.find(token.value) == OPERATORINFOMAP.end()) {
                        throw ExpressionException("Unknown operator during evaluation: " + token.value);
                    }
                    
                    OperatorInfo info = OPERATORINFOMAP.at(token.value);

                    evalStack.push(info.apply(first, second));
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

    std::function<double(const std::vector<double>&, const std::map<std::string, double>&)> ExpressionParser::parse() {

        if (!validateExpr()) {
            throw ExpressionException("Invalid expression: " + expression);
        }

        std::vector<Token> sortedTokens = {};
        std::stack<Token> operatorStack = {};

        // Shunting Yard Algorithm
        shantingYardAlgo(sortedTokens, operatorStack);

        // Pop any remaining operators from the stack to the output queue
        while (!operatorStack.empty()) {
            sortedTokens.push_back(operatorStack.top());
            operatorStack.pop();
        }

        return buildEvaluator(std::move(sortedTokens));
    }

    bool ExpressionParser::validateExpr() const {
        // std::vector<Token> tokens = this->getTokens();

        Token lastToken = Token{TokenType::Unknown, ""};

        std::stack<Token> parenthesesStack;

        for (const auto& token: tokens) {
            if (token.type == TokenType::Variable){
                if (!std::isalpha(token.value.at(0)) && token.value.at(0) != '_') {
                    return false;
                }

                // Can't follow a variable, literal, or another index
                if (isOperand(lastToken)) {
                    return false;
                }

            } else if (token.type == TokenType::Literal) {
                try {
                    std::stod(token.value);
                } catch (const std::exception& e) {
                    return false;
                }

                // Can't follow a variable, literal, or another index
                if (isOperand(lastToken)) {
                    return false;
                }

            } else if (token.type == TokenType::Index) {
                try {
                    std::stoi(token.value);
                } catch (const std::exception& e) {
                    return false;
                }

                // Can't follow a variable, literal, or another index
                if (isOperand(lastToken)) {
                    return false;
                }

            } else if (isOperator(token)) {

                // Can't follow an operator, a left parenthesis, or be the first token
                if (isOperator(lastToken) || isLeftParen(lastToken) || lastToken.type == TokenType::Unknown) {
                    return false;
                }

            } else if (isLeftParen(token)) {
                // Can't follow an operand or a right parenthesis
                if (isOperand(lastToken) || isRightParen(lastToken)) {
                    return false;
                }
                parenthesesStack.push(token);

            } else if (isRightParen(token)) {
                // Can't follow an operator, a left parenthesis, or be the first token
                if (isOperator(lastToken) || isLeftParen(lastToken) || lastToken.type == TokenType::Unknown) {
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
            } else {
                return false;

            }

            lastToken = token;
        }

        // Expr can't be empty, end with an operator, or have unbalanced parentheses
        if (isOperator(lastToken) || lastToken.type == TokenType::Unknown || !parenthesesStack.empty()) {
            return false;
        }

        return true;
    }

    void ExpressionParser::tokenize() {
        tokens.clear();

        unsigned i = 0;

        while (i < expression.size()) {
            char c = expression[i];

            if (std::isspace(c)) {
                ++i;
                continue;

            } else if (std::isalpha(c)) {
                // Extract a variable token, which starts with a letter and can be followed by letters, digits, or underscores
                std::string var = "";
                var += c;

                while (i + 1 < expression.size() && (std::isalnum(expression.at(i + 1)) || expression.at(i + 1) == '_')) {
                    var += expression.at(++i);
                }

                Token token{TokenType::Variable, var};
                tokens.push_back(token);
                variables.insert(var);
                tokenStrings += token.value + " ";

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
                tokenStrings += literal + " ";

            } else if (c == '{') {
                // Extract everything until the closing brace as an index token
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
                tokenStrings += "{" + index + "} ";

            } else if (c == '(' || c == ')') {
                // Parentheses are tokens on their own
                Token token{TokenType::Parenthesis, std::string(1, c)};
                tokens.push_back(token);
                tokenStrings += token.value + " ";

            } else if (c == '+' || c == '-' || c == '*' || c == '/' || 
                        c == '^' || c == '%') {
                // Sinle-character operators
                Token token{TokenType::Operator, std::string(1, c)};
                tokens.push_back(token);
                tokenStrings += token.value + " ";

            } else if (c == '<' || c == '>') {
                // Two-character operators: <=, >=
                if (i + 1 < expression.size() && expression.at(i + 1) == '=') {
                    Token token{TokenType::Operator, std::string(1, c) + "="};
                    tokens.push_back(token);
                    ++i;
                } else {
                    Token token{TokenType::Operator, std::string(1, c)};
                    tokens.push_back(token);
                }

                tokenStrings += tokens.back().value + " ";
                
            } else if (c == '=' || c == '&' || c == '|') {
                // Two-character operators: ==, &&, ||
                if (i + 1 < expression.size() && expression.at(i + 1) == c) {
                    tokens.push_back(Token{TokenType::Operator, std::string(2, c)});
                    ++i;
                } else {
                    throw ExpressionException(std::string("Invalid operator: Single '") + c + "' not supported");
                }

                tokenStrings += tokens.back().value + " ";

            } else if (c == '!') {
                // Two-character operator: !=
                if (i + 1 < expression.size() && expression.at(i + 1) == '=') {
                    Token token{TokenType::Operator, "!="};
                    tokens.push_back(token);
                    ++i;
                } else {
                    throw ExpressionException("Invalid operator: Single '!' not supported");
                }

                tokenStrings += tokens.back().value + " ";

            } else {
                throw ExpressionException(std::string("Invalid character in expression: '") + c + "'");
            }

            ++i;
        }

        return;
    }
}