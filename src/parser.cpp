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

// In src/parser.cpp

std::unique_ptr<StatementNode> Parser::parseStatement() {
    if (check(TokenType::RETURN)) {
        return parseReturnStatement();
    }
    if (check(TokenType::AUTO)) {
        return parseAutoStatement();
    }

    // --- NEW, SMARTER LOGIC ---
    // Check for the "identifier followed by a parenthesis" pattern
    // to disambiguate function calls from other potential statements.
    if (check(TokenType::IDENTIFIER) && m_tokens[m_current + 1].type == TokenType::LEFT_PAREN) {
        // This is a function call that is being used as a standalone statement
        // (e.g., `print(x);`). Its value is discarded.

        // We can parse it as an expression and wrap it in a statement node,
        // or just call parseFunctionCallStatement directly. Let's do the latter for clarity.
        return parseFunctionCallStatement();
    }

    // If you were to add assignment statements like `x = 5;`, you would add another check here:
    // if (check(TokenType::IDENTIFIER) && m_tokens[m_current + 1].type == TokenType::EQUAL) {
    //     return parseAssignmentStatement();
    // }

    // If we get here, we have a token we don't know how to start a statement with.
    std::cerr << "Parse Error: Invalid start of a statement. Found token '" << peek().value << "'\n";
    return nullptr;
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

std::unique_ptr<StatementNode> Parser::parseAutoStatement() {
    // 1. Consume the 'auto' keyword. We already know it's there from parseStatement.
    consume(TokenType::AUTO, "Expect 'auto' keyword."); // This just advances the token stream

    // 2. Create the AST node to hold the data
    auto autoNode = std::make_unique<AutoStatementNode>();

    // 3. Parse the variable name (it must be an identifier)
    if (!consume(TokenType::IDENTIFIER, "Expect variable name after 'auto'.")) return nullptr;
    autoNode->name = previous(); // 'previous()' gives us the token we just consumed

    // 4. Parse the equals sign. This was the part you knew was missing!
    if (!consume(TokenType::EQUAL, "Expect '=' after variable name.")) return nullptr;

    // 5. Parse the initializer expression
    autoNode->initializer = parseExpression();
    if (!autoNode->initializer) return nullptr; // Check for parsing errors in the expression

    // 6. Parse the final semicolon
    if (!consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.")) return nullptr;

    // 7. Success! Return the completed AST node.
    return autoNode;
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

// Add this to parser.cpp
std::unique_ptr<ExpressionNode> Parser::parseFunctionCallExpression() {
    // Create the AST Node (you'll need to define FunctionCallExprNode in ast.hpp)
    auto funcCall = std::make_unique<FunctionCallExpressionNode>();

    // The logic is the same as parseFunctionCallStatement, but without the trailing semicolon.
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

    // --- THIS IS THE KEY FIX ---
    if (check(TokenType::IDENTIFIER)) {
        // We see an identifier. Is it a variable OR a function call?
        // Let's PEEK ahead one token. We don't consume it yet.
        if (m_tokens[m_current + 1].type == TokenType::LEFT_PAREN) {
            // It's an identifier followed by '(', so it MUST be a function call.
            return parseFunctionCallExpression(); // We need to write this function!
        } else {
            // It's just a variable name.
            auto varNode = std::make_unique<VariableNode>();
            varNode->name = advance();
            return varNode;
        }
    }

    if (check(TokenType::LEFT_PAREN)) {
        advance();
        auto expr = parseExpression();
        if (!consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.")) return nullptr;
        return expr;
    }

    std::cerr << "Parse Error: Expected an expression..." << std::endl;
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

