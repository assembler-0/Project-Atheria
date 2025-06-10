#include "ast_printer.hpp"
#include <iostream>

void AstPrinter::print(AstNode* node) {
    node->accept(*this);
}

void AstPrinter::indent() { m_indent_level++; }
void AstPrinter::dedent() { m_indent_level--; }

std::string currentIndent(int level, const std::string& str) {
    std::string total_indent = "";
    for (int i = 0; i < level; ++i) total_indent += str;
    return total_indent;
}

void AstPrinter::visit(ProgramNode* node) {
    std::cout << currentIndent(m_indent_level, m_indent_str) << "Program\n";
    indent();
    for (const auto& func : node->functions) {
        func->accept(*this);
    }
    dedent();
}

void AstPrinter::visit(FunctionDefinitionNode* node) {
    std::cout << currentIndent(m_indent_level, m_indent_str)
              << "FunctionDef(name=" << node->functionName.value
              << ", returns=" << node->returnType.value << ")\n";
    indent();
    for (const auto& stmt : node->body) {
        stmt->accept(*this);
    }
    dedent();
}

void AstPrinter::visit(FunctionCallStatementNode* node) {
    std::cout << currentIndent(m_indent_level, m_indent_str)
              << "FunctionCall(name=" << node->functionName.value << ")\n";
    indent();
    std::cout << currentIndent(m_indent_level, m_indent_str) << "Arguments:\n";
    indent();
    for (const auto& arg : node->arguments) {
        arg->accept(*this);
    }
    dedent();
    dedent();
}

void AstPrinter::visit(StringLiteralNode* node) {
    std::cout << currentIndent(m_indent_level, m_indent_str)
              << "StringLiteral(value=\"" << node->value.value << "\")\n";
}