#pragma once
#include "ast.hpp"
#include <memory>

// --- FIX ---
// We include the full headers here, which is the correct and simple way.
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

// --- REMOVED ---
// The manual forward declaration block below was conflicting with the includes above.
// It has been deleted.

class CodeGen : public AstVisitor {
public:
    CodeGen();

    void generate(ProgramNode* program);
    void dump();
    void emitObjectFile(const std::string& filename);

private:
    // Visitor Methods - these are correct with 'override'
    void visit(ProgramNode* node) override;
    void visit(FunctionDefinitionNode* node) override;
    void visit(FunctionCallStatementNode* node) override;
    void visit(StringLiteralNode* node) override;

    // Core LLVM objects - these are now correct
    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::Module> m_module;
    std::unique_ptr<llvm::IRBuilder<>> m_builder; // This now works

    // Helper member for passing values
    llvm::Value* m_last_value = nullptr;
};