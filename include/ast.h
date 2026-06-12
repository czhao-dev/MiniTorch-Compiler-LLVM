#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace minitorch {

struct TensorTypeNode {
    std::vector<int> dims;
};

struct ParamNode {
    std::string name;
    TensorTypeNode type;
};

enum class ExprKind {
    Call,
    Binary,
    Ident,
    Number,
};

struct ExprNode {
    explicit ExprNode(ExprKind kind) : kind(kind) {}
    virtual ~ExprNode() = default;

    ExprKind kind;
};

struct IdentExprNode final : ExprNode {
    explicit IdentExprNode(std::string name) : ExprNode(ExprKind::Ident), name(std::move(name)) {}
    std::string name;
};

struct NumberExprNode final : ExprNode {
    explicit NumberExprNode(double value) : ExprNode(ExprKind::Number), value(value) {}
    double value;
};

enum class BinaryOp {
    Add,
    Mul,
};

struct BinOpExprNode final : ExprNode {
    BinOpExprNode(BinaryOp op, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right)
        : ExprNode(ExprKind::Binary), op(op), left(std::move(left)), right(std::move(right)) {}

    BinaryOp op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
};

struct CallExprNode final : ExprNode {
    CallExprNode(std::string callee, std::vector<std::unique_ptr<ExprNode>> args)
        : ExprNode(ExprKind::Call), callee(std::move(callee)), args(std::move(args)) {}

    std::string callee;
    std::vector<std::unique_ptr<ExprNode>> args;
};

enum class StmtKind {
    Assign,
    Return,
};

struct StmtNode {
    explicit StmtNode(StmtKind kind) : kind(kind) {}
    virtual ~StmtNode() = default;

    StmtKind kind;
};

struct AssignStmtNode final : StmtNode {
    AssignStmtNode(std::string target, std::unique_ptr<ExprNode> value)
        : StmtNode(StmtKind::Assign), target(std::move(target)), value(std::move(value)) {}

    std::string target;
    std::unique_ptr<ExprNode> value;
};

struct ReturnStmtNode final : StmtNode {
    explicit ReturnStmtNode(std::unique_ptr<ExprNode> value)
        : StmtNode(StmtKind::Return), value(std::move(value)) {}

    std::unique_ptr<ExprNode> value;
};

struct FuncDefNode {
    std::string decorator;
    std::string name;
    std::vector<ParamNode> params;
    TensorTypeNode returnType;
    std::vector<std::unique_ptr<StmtNode>> body;
    std::size_t line = 0;
};

struct ModuleNode {
    std::vector<std::unique_ptr<FuncDefNode>> functions;
};

void printAst(const ModuleNode &module, std::ostream &out);

} // namespace minitorch

