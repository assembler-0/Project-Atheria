#include "codegen.hpp"
#include <iostream>
#include <optional>
// All the LLVM includes needed for the whole process
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"

CodeGen::CodeGen() {
    m_context = std::make_unique<llvm::LLVMContext>();
    m_module = std::make_unique<llvm::Module>("AtheriaModule", *m_context);
    m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);
}

void CodeGen::generate(ProgramNode* program) {
    program->accept(*this);
}

void CodeGen::dump() {
    m_module->print(llvm::errs(), nullptr);
}

// --- Visitor Implementations ---

void CodeGen::visit(ProgramNode* node) {
    for (const auto& func : node->functions) {
        func->accept(*this);
    }
}

void CodeGen::visit(FunctionDefinitionNode* node) {
    if (node->returnType.value != "int32_t") {
        // --- FIX: Replaced throw with cerr ---
        std::cerr << "CodeGen Error: Unsupported return type '" << node->returnType.value << "'\n";
        return;
    }
    llvm::Type* returnType = m_builder->getInt32Ty();
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->functionName.value, m_module.get());

    llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_context, "entry", func);
    m_builder->SetInsertPoint(block);

    for (const auto& stmt : node->body) {
        stmt->accept(*this);
    }

    m_builder->CreateRet(m_builder->getInt32(0));
    llvm::verifyFunction(*func);
}

void CodeGen::visit(FunctionCallStatementNode* node) {
    if (node->functionName.value == "print") {
        llvm::Function* printf_func = m_module->getFunction("printf");

        if (!printf_func) {
            llvm::PointerType* printf_arg_type = m_builder->getPtrTy();
            llvm::FunctionType* printf_type = llvm::FunctionType::get(m_builder->getInt32Ty(), printf_arg_type, true);
            printf_func = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", m_module.get());
        }

        if (node->arguments.empty()) {
            // --- FIX: Replaced throw with cerr ---
            std::cerr << "CodeGen Error: 'print' function requires one argument.\n";
            return;
        }
        node->arguments[0]->accept(*this);
        llvm::Value* format_str = m_last_value;

        m_builder->CreateCall(printf_func, format_str);
        m_last_value = nullptr;
        return;
    }

    // --- FIX: Replaced throw with cerr ---
    std::cerr << "CodeGen Error: Unknown function called '" << node->functionName.value << "'\n";
}

void CodeGen::visit(StringLiteralNode* node) {
    std::string format_str = node->value.value + "\n";
    m_last_value = m_builder->CreateGlobalStringPtr(format_str, "str_literal");
}


// --- Boilerplate for creating the final .o file ---
void CodeGen::emitObjectFile(const std::string& filename) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    m_module->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!target) {
        llvm::errs() << error;
        return;
    }

    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    // THE FIX: Explicitly set the Relocation Model to Position-Independent Code.
    auto rm = llvm::Reloc::Model::PIC_;
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, rm);

    m_module->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Could not open file: " << ec.message();
        return;
    }

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        llvm::errs() << "The TargetMachine can't emit a file of this type.";
        return;
    }

    pass.run(*m_module);
    dest.flush();
    std::cout << "Successfully wrote object file to '" << filename << "'\n";
}