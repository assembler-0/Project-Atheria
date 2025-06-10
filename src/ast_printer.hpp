#pragma once
#include "ast.hpp"
#include <string>

class AstPrinter : public AstVisitor {
public:
    void print(AstNode* node);

    void visit(ProgramNode* node) override;
    void visit(FunctionDefinitionNode* node) override;
    void visit(FunctionCallStatementNode* node) override;
    void visit(StringLiteralNode* node) override;

private:
    void indent();
    void dedent();

    int m_indent_level = 0;
    std::string m_indent_str = "  "; // 2 spaces per indent level
};