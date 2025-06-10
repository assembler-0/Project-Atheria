#include "parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program = std::make_unique<ProgramNode>();
    while (!isAtEnd()) {
        auto funcDef = parseFunctionDefinition();
        if (!funcDef) return nullptr;
        program->functions.push_back(std::move(funcDef));
    }
    return program;
}

std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    auto funcDef = std::make_unique<FunctionDefinitionNode>();
    if (!consume(TokenType::IDENTIFIER, "Expect return type.")) return nullptr;
    funcDef->returnType = previous();
    if (!consume(TokenType::IDENTIFIER, "Expect function name.")) return nullptr;
    funcDef->functionName = previous();
    if (!consume(TokenType::LEFT_PAREN, "Expect '(' after function name.")) return nullptr;

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            auto param = parseParameter();
            if (!param) return nullptr;
            funcDef->parameters.push_back(std::move(param));
        } while (consume(TokenType::COMMA, ""));
    }

    if (!consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.")) return nullptr;
    if (!consume(TokenType::LEFT_BRACE, "Expect '{' before function body.")) return nullptr;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (!stmt) return nullptr;
        funcDef->body.push_back(std::move(stmt));
    }

    if (!consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.")) return nullptr;
    return funcDef;
}

std::unique_ptr<ParameterNode> Parser::parseParameter() {
    auto param = std::make_unique<ParameterNode>();
    if (!consume(TokenType::IDENTIFIER, "Expect parameter type.")) return nullptr;
    param->type = previous();
    if (!consume(TokenType::IDENTIFIER, "Expect parameter name.")) return nullptr;
    param->name = previous();
    return param;
}

std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (check(TokenType::RETURN)) {
        return parseReturnStatement();
    }
    // You can add more later, e.g., if (check(TokenType::LET)) { return parseLetStatement(); }

    // Default to a function call if it's an identifier followed by a parenthesis
    // This part is a little tricky, but let's assume for now the only other
    // statement is a function call. A more robust parser would check this better.
    return parseFunctionCallStatement();
}

// Add this new function to parser.cpp
std::unique_ptr<StatementNode> Parser::parseReturnStatement() {
    // 1. Consume the 'return' keyword
    if (!consume(TokenType::RETURN, "Expect 'return' keyword.")) return nullptr;

    // 2. Create the AST node
    auto returnNode = std::make_unique<ReturnStatementNode>();

    // 3. Parse the expression that comes after 'return'
    returnNode->returnValue = parseExpression();
    if (!returnNode->returnValue) {
        // Error already printed by parseExpression
        return nullptr;
    }

    // 4. Consume the trailing semicolon
    if (!consume(TokenType::SEMICOLON, "Expect ';' after return value.")) return nullptr;

    return returnNode;
}

std::unique_ptr<FunctionCallStatementNode> Parser::parseFunctionCallStatement() {
    auto funcCall = std::make_unique<FunctionCallStatementNode>();
    if (!consume(TokenType::IDENTIFIER, "Expect function name for call.")) return nullptr;
    funcCall->functionName = previous();
    if (!consume(TokenType::LEFT_PAREN, "Expect '(' after function name.")) return nullptr;

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            auto arg = parseExpression();
            if (!arg) return nullptr;
            funcCall->arguments.push_back(std::move(arg));
        } while (consume(TokenType::COMMA, ""));
    }

    if (!consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.")) return nullptr;
    if (!consume(TokenType::SEMICOLON, "Expect ';' after statement.")) return nullptr;
    return funcCall;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() { return parseTerm(); }

std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        auto right = parseFactor();
        if (!left || !right) return nullptr;
        auto new_left = std::make_unique<BinaryOpNode>();
        new_left->left = std::move(left);
        new_left->op = op;
        new_left->right = std::move(right);
        left = std::move(new_left);
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    auto left = parsePrimary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token op = advance();
        auto right = parsePrimary();
        if (!left || !right) return nullptr;
        auto new_left = std::make_unique<BinaryOpNode>();
        new_left->left = std::move(left);
        new_left->op = op;
        new_left->right = std::move(right);
        left = std::move(new_left);
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    if (check(TokenType::NUMBER_LITERAL)) {
        auto numNode = std::make_unique<NumberLiteralNode>();
        numNode->value = advance();
        return numNode;
    }
    if (check(TokenType::STRING_LITERAL)) {
        auto strNode = std::make_unique<StringLiteralNode>();
        strNode->value = advance();
        return strNode;
    }
    if (check(TokenType::IDENTIFIER)) {
        auto varNode = std::make_unique<VariableNode>();
        varNode->name = advance();
        return varNode;
    }
    if (check(TokenType::LEFT_PAREN)) {
        advance();
        auto expr = parseExpression();
        if (!consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.")) return nullptr;
        return expr;
    }
    std::cerr << "Parse Error: Expected an expression, but found token '" << peek().value << "'." << std::endl;
    return nullptr;
}

Token Parser::peek() { return m_tokens[m_current]; }
Token Parser::previous() { return m_tokens[m_current - 1]; }
bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
bool Parser::check(TokenType type) { if(isAtEnd()) return false; return peek().type == type; }
Token Parser::advance() { if (!isAtEnd()) m_current++; return previous(); }

bool Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return true;
    }
    if (!message.empty()) {
        std::cerr << "Parse error: " << message << std::endl;
    }
    return false;
}

