#include "codegen.hpp"
#include <iostream>

// All the necessary LLVM headers for the whole process
#include "llvm/IR/Verifier.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/IR/LegacyPassManager.h"

CodeGen::CodeGen() {
    // Initialize the core LLVM components
    m_context = std::make_unique<llvm::LLVMContext>();
    m_module = std::make_unique<llvm::Module>("AtheriaModule", *m_context);
    m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);
}

// A helper function to convert our language's type names into LLVM's type objects
llvm::Type* CodeGen::getLlvmType(const Token& token) {
    if (token.value == "int32_t") {
        return m_builder->getInt32Ty();
    }
    // You can add more types like "float", "bool", etc. here later
    std::cerr << "CodeGen Error: Unknown type '" << token.value << "'\n";
    return nullptr;
}

// The main entry point for the code generator
void CodeGen::generate(ProgramNode* program) {
    program->accept(*this);
}

// --- Visitor Implementations: Where the Magic Happens ---

void CodeGen::visit(ProgramNode* node) {
    // A program is just a list of functions, so we visit each one.
    for (const auto& func : node->functions) {
        func->accept(*this);
    }
}

void CodeGen::visit(FunctionDefinitionNode* node) {
    // ---- 1. SETUP ----
    // Clear the symbol table for this new function's scope. This is crucial
    // so that variables from one function don't leak into another.
    m_symbol_table.clear();

    // ---- 2. CREATE FUNCTION SIGNATURE ----
    // Collect the LLVM types of the parameters
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node->parameters) {
        llvm::Type* type = getLlvmType(param->type);
        if (!type) return; // Error was already printed by getLlvmType
        paramTypes.push_back(type);
    }

    // Get the return type
    llvm::Type* returnType = getLlvmType(node->returnType);
    if (!returnType) return;

    // Create the actual LLVM function type and function object
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->functionName.value, m_module.get());

    // ---- 3. CREATE FUNCTION BODY ----
    // Create the "entry" block for the function and tell the IR builder to start writing code here
    llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_context, "entry", func);
    m_builder->SetInsertPoint(block);

    // ---- 4. PROCESS PARAMETERS ----
    // Now we handle the incoming arguments, giving them names and storing them
    // on the stack so they can be used like regular variables.
    auto param_it = node->parameters.begin();
    for (auto& arg : func->args()) {
        arg.setName((*param_it)->name.value);

        // Create a mutable variable on the stack (an "alloca") for the parameter
        llvm::Type* paramLlvmType = getLlvmType((*param_it)->type);
        llvm::Value* alloca = m_builder->CreateAlloca(paramLlvmType, nullptr, arg.getName());

        // Store the initial argument value into our new stack variable
        m_builder->CreateStore(&arg, alloca);

        // Add the stack variable to our symbol table so we can find it by name later
        m_symbol_table[std::string(arg.getName())] = alloca;

        param_it++;
    }

    // ---- 5. GENERATE CODE FOR STATEMENTS ----
    // Visit each statement in the function's body
    for (const auto& stmt : node->body) {
        stmt->accept(*this);
    }

    // ---- 6. VERIFICATION ----
    // Ask LLVM to verify that our generated function is valid. This catches many bugs.
    llvm::verifyFunction(*func);
}


// A NumberLiteral simply becomes an LLVM integer constant.
void CodeGen::visit(NumberLiteralNode* node) {
    int val = std::stoi(node->value.value);
    m_last_value = m_builder->getInt32(val);
}

// A StringLiteral becomes a global constant string pointer.
void CodeGen::visit(StringLiteralNode* node) {
    m_last_value = m_builder->CreateGlobalStringPtr(node->value.value, "str_literal");
}


// To use a variable, we find it in the symbol table and load its value from the stack.
void CodeGen::visit(VariableNode* node) {
    // Look up the variable's memory location (the AllocaInst*) in the symbol table.
    llvm::Value* ptr = m_symbol_table[node->name.value];
    if (!ptr) {
        std::cerr << "CodeGen Error: Unknown variable name '" << node->name.value << "'\n";
        m_last_value = nullptr;
        return;
    }
    // An AllocaInst* is a pointer. We need to get the type it's pointing to.
    llvm::Type* type = static_cast<llvm::AllocaInst*>(ptr)->getAllocatedType();

    // `CreateLoad` generates the instruction to load the value from memory.
    m_last_value = m_builder->CreateLoad(type, ptr, node->name.value);
}

// For a binary operation, we generate code for both sides, then create the final instruction.
void CodeGen::visit(BinaryOpNode* node) {
    // Recursively generate code for the left and right hand sides
    node->left->accept(*this);
    llvm::Value* L = m_last_value;

    node->right->accept(*this);
    llvm::Value* R = m_last_value;

    if (!L || !R) {
        m_last_value = nullptr;
        return;
    }

    // Create the correct LLVM instruction based on the operator token
    switch (node->op.type) {
        case TokenType::PLUS:
            m_last_value = m_builder->CreateAdd(L, R, "addtmp");
            break;
        case TokenType::MINUS:
            m_last_value = m_builder->CreateSub(L, R, "subtmp");
            break;
        case TokenType::STAR:
            m_last_value = m_builder->CreateMul(L, R, "multmp");
            break;
        case TokenType::SLASH:
            m_last_value = m_builder->CreateSDiv(L, R, "divtmp"); // SDiv = Signed Divide
            break;
        default:
            std::cerr << "CodeGen Error: Invalid binary operator\n";
            m_last_value = nullptr;
    }
}


// Calling a function is complex because we need to handle different argument types.
void CodeGen::visit(FunctionCallStatementNode* node) {
    // The only built-in function we have right now is 'print'
    if (node->functionName.value == "print") {
        // Look up the C 'printf' function, or declare it if it doesn't exist
        llvm::Function* printf_func = m_module->getFunction("printf");
        if (!printf_func) {
            llvm::IntegerType* printf_arg_type = m_builder->getIntPtrTy();
            llvm::FunctionType* printf_type = llvm::FunctionType::get(m_builder->getInt32Ty(), printf_arg_type, true);
            printf_func = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", m_module.get());
        }

        if (node->arguments.empty()) {
            std::cerr << "CodeGen Error: 'print' function requires one argument.\n";
            return;
        }

        // Generate the code for the argument expression
        node->arguments[0]->accept(*this);
        llvm::Value* arg_value = m_last_value;

        // --- NEW: Handle printing integers vs strings ---
        std::vector<llvm::Value*> printf_args;
        if (arg_value->getType()->isPointerTy()) {
            // It's a string. We need the format string "%s\n"
            llvm::Value* format_str = m_builder->CreateGlobalStringPtr("%s\n", "fmt_str_s");
            printf_args.push_back(format_str);
            printf_args.push_back(arg_value);
        } else if (arg_value->getType()->isIntegerTy()) {
            // It's an integer. We need the format string "%d\n"
            llvm::Value* format_str = m_builder->CreateGlobalStringPtr("%d\n", "fmt_str_d");
            printf_args.push_back(format_str);
            printf_args.push_back(arg_value);
        } else {
             std::cerr << "CodeGen Error: 'print' can only handle strings and integers for now.\n";
             return;
        }

        m_builder->CreateCall(printf_func, printf_args);
        m_last_value = nullptr; // A print statement produces no value
        return;
    }

    std::cerr << "CodeGen Error: Unknown function called '" << node->functionName.value << "'\n";
}

void CodeGen::visit(ReturnStatementNode* node) {
    node->expression->accept(*this); // Visit the expression to get its value
    m_builder->CreateRet(m_last_value); // Create the return instruction
}
// --- Boilerplate and Debugging ---

void CodeGen::dump() {
    m_module->print(llvm::errs(), nullptr);
}

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