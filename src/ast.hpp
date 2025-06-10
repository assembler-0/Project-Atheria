#pragma once
#include <string>
#include <vector>
#include <memory>
#include "token.hpp" // We need the Token struct

// Forward-declare all our node types so they can know about each other
struct ProgramNode;
struct FunctionDefinitionNode;
struct StatementNode;
struct ExpressionNode;
struct FunctionCallStatementNode;
struct StringLiteralNode;


// --- Visitor Pattern ---
// This is a clean way to process AST nodes without cluttering the node classes themselves.
// We'll use it for our AstPrinter, and later for the Code Generator.
struct AstVisitor {
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(FunctionDefinitionNode* node) = 0;
    virtual void visit(FunctionCallStatementNode* node) = 0;
    virtual void visit(StringLiteralNode* node) = 0;
};


// --- Base Node Types ---
// All nodes in our AST will inherit from AstNode.
struct AstNode {
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor& visitor) = 0;
};

// Base class for all "statement" nodes (actions that don't produce a value)
struct StatementNode : public AstNode {};

// Base class for all "expression" nodes (things that produce a value)
struct ExpressionNode : public AstNode {};


// --- Concrete Node Types ---

struct StringLiteralNode : public ExpressionNode {
    Token value; // The STRING_LITERAL token

    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct FunctionCallStatementNode : public StatementNode {
    Token functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;

    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct FunctionDefinitionNode : public AstNode {
    Token returnType;
    Token functionName;
    std::vector<std::unique_ptr<StatementNode>> body;

    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

// The root of our entire tree
struct ProgramNode : public AstNode {
    std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;

    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};