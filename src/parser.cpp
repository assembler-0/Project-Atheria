#include "parser.hpp"
#include <stdexcept> // For std::runtime_error

Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens) {}

// The main parsing function
std::unique_ptr<ProgramNode> Parser::parse() {
    auto program = std::make_unique<ProgramNode>();
    while (!isAtEnd()) {
        program->functions.push_back(parseFunctionDefinition());
    }
    return program;
}

// --- Grammar Rule Parsers ---

// functionDefinition -> type identifier "(" ")" "{" statement* "}"
std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    auto funcDef = std::make_unique<FunctionDefinitionNode>();
    funcDef->returnType = consume(TokenType::IDENTIFIER, "Expect return type.");
    funcDef->functionName = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        funcDef->body.push_back(parseStatement());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after function body.");
    return funcDef;
}

// statement -> functionCallStatement
std::unique_ptr<StatementNode> Parser::parseStatement() {
    // For now, the only statement we support is a function call
    return parseFunctionCallStatement();
}

// functionCallStatement -> identifier "(" expression? ")" ";"
std::unique_ptr<FunctionCallStatementNode> Parser::parseFunctionCallStatement() {
    auto funcCall = std::make_unique<FunctionCallStatementNode>();
    funcCall->functionName = consume(TokenType::IDENTIFIER, "Expect function name for call.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    // Parse arguments if they exist
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            funcCall->arguments.push_back(parseExpression());
        } while (false); // No commas for now, just one argument
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    consume(TokenType::SEMICOLON, "Expect ';' after statement.");
    return funcCall;
}

// expression -> STRING_LITERAL
std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    if (check(TokenType::STRING_LITERAL)) {
        auto strLiteral = std::make_unique<StringLiteralNode>();
        strLiteral->value = advance();
        return strLiteral;
    }

    throw std::runtime_error("Parse error: Expected an expression.");
}


// --- Helper Methods ---

Token Parser::peek() { return m_tokens[m_current]; }
Token Parser::previous() { return m_tokens[m_current - 1]; }
bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
bool Parser::check(TokenType type) { return peek().type == type; }

Token Parser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error("Parse error: " + message);
}