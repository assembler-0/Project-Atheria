#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vector>
#include "token.hpp" // We need the Token struct


struct ProgramNode;
struct FunctionDefinitionNode;
struct StatementNode;
struct ExpressionNode;
struct FunctionCallStatementNode;
struct StringLiteralNode;
struct NumberLiteralNode;
struct BinaryOpNode;
struct VariableNode;
struct ParameterNode;
struct ReturnStatementNode;
struct AutoStatementNode;
struct FunctionCallExpressionNode;
// --- Visitor Pattern ---
// This is a clean way to process AST nodes without cluttering the node classes themselves.
// We'll use it for our AstPrinter, and later for the Code Generator.
struct AstVisitor {
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(FunctionDefinitionNode* node) = 0;
    virtual void visit(FunctionCallStatementNode* node) = 0;
    virtual void visit(StringLiteralNode* node) = 0;
    virtual void visit(NumberLiteralNode* node) = 0;
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(ReturnStatementNode* node) = 0;
    virtual void visit(AutoStatementNode* node) = 0;
    virtual void visit(FunctionCallExpressionNode* node) = 0;
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
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct ParameterNode : public AstNode {
    Token type;
    Token name;
    // This is no longer an override, but it's cleaner to remove it.
    // void accept(AstVisitor& visitor) override { visitor.visit(this); } // <-- DELETE THIS LINE
    // Let's make it an error to call accept on it.
    void accept(AstVisitor& visitor) override {
        // This should never be called. Parameters are handled directly by FunctionDefinitionNode's visitor.
        (void)visitor; // a way to tell the compiler we are intentionally not using a parameter
    }
};
// The root of our entire tree
struct ProgramNode : public AstNode {
    std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;

    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};


struct NumberLiteralNode : public ExpressionNode {
    Token value; // The NUMBER_LITERAL token
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct VariableNode : public ExpressionNode {
    Token name; // The IDENTIFIER token
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct BinaryOpNode : public ExpressionNode {
    std::unique_ptr<ExpressionNode> left;
    Token op; // The operator token (+, -, *, /)
    std::unique_ptr<ExpressionNode> right;
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct ReturnStatementNode : public StatementNode {
    std::unique_ptr<ExpressionNode> expression;
    std::unique_ptr<ExpressionNode> returnValue;
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct AutoStatementNode : public StatementNode{
    Token name;
    std::unique_ptr<ExpressionNode> initializer;
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};

struct FunctionCallExpressionNode : public ExpressionNode{
    Token functionName;
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    void accept(AstVisitor& visitor) override { visitor.visit(this); }
};