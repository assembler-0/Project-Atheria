#pragma once
#include "ast.hpp"
#include <memory>
#include <map> // <-- NEW: For our symbol table

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

class CodeGen : public AstVisitor {
public:
    CodeGen();
    void generate(ProgramNode* program);
    void dump();
    void emitObjectFile(const std::string& filename);

private:
    // Visitor Methods for all our AST nodes
    void visit(ProgramNode* node) override;
    void visit(FunctionDefinitionNode* node) override;
    void visit(FunctionCallStatementNode* node) override;
    void visit(StringLiteralNode* node) override;
    void visit(NumberLiteralNode* node) override; // <-- NEW
    void visit(BinaryOpNode* node) override;      // <-- NEW
    void visit(VariableNode* node) override;      // <-- NEW
    void visit(ReturnStatementNode* node) override;
    void visit(AutoStatementNode* node) override;
    void visit(FunctionCallExpressionNode* node) override;

    // --- Core LLVM Objects ---
    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::Module> m_module;
    std::unique_ptr<llvm::IRBuilder<>> m_builder;

    // --- NEW: Symbol Table ---
    // Maps a variable name (string) to its memory location (Value*).
    std::map<std::string, llvm::Value*> m_symbol_table;

    // Helper member for passing values from expressions
    llvm::Value* m_last_value = nullptr;

    // Helper to get LLVM type from our type names
    llvm::Type* getLlvmType(const Token& token);
};