#pragma once
#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> m_tokens;
    size_t m_current = 0;

    // Helper methods
    Token peek();
    Token previous();
    Token advance();
    bool isAtEnd();
    bool check(TokenType type);
    bool consume(TokenType type, const std::string& message);

    // Parsing methods
    std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
    std::unique_ptr<StatementNode> parseStatement();
    std::unique_ptr<StatementNode> parseReturnStatement();
    std::unique_ptr<FunctionCallStatementNode> parseFunctionCallStatement();

    // --- Expression Parsing Hierarchy ---
    std::unique_ptr<ExpressionNode> parseExpression(); // Entry Point
    std::unique_ptr<ExpressionNode> parseTerm();       // Handles: + -
    std::unique_ptr<ExpressionNode> parseFactor();     // Handles: * /
    std::unique_ptr<ExpressionNode> parsePrimary();    // Handles: Literals, Grouping
    std::unique_ptr<ParameterNode> parseParameter();
};