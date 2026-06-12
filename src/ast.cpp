#include "ast.h"

#include <iomanip>

namespace minitorch {

namespace {

void printIndent(std::ostream &out, int indent) {
    for (int i = 0; i < indent; ++i) {
        out << "  ";
    }
}

void printTensorType(const TensorTypeNode &type, std::ostream &out) {
    out << "Tensor[";
    for (std::size_t i = 0; i < type.dims.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << type.dims[i];
    }
    out << "]";
}

void printExpr(const ExprNode &expr, std::ostream &out, int indent) {
    printIndent(out, indent);
    switch (expr.kind) {
    case ExprKind::Ident: {
        const auto &ident = static_cast<const IdentExprNode &>(expr);
        out << "Ident(" << ident.name << ")\n";
        break;
    }
    case ExprKind::Number: {
        const auto &number = static_cast<const NumberExprNode &>(expr);
        out << "Number(" << std::setprecision(8) << number.value << ")\n";
        break;
    }
    case ExprKind::Binary: {
        const auto &binary = static_cast<const BinOpExprNode &>(expr);
        out << "Binary(" << (binary.op == BinaryOp::Add ? "+" : "*") << ")\n";
        printExpr(*binary.left, out, indent + 1);
        printExpr(*binary.right, out, indent + 1);
        break;
    }
    case ExprKind::Call: {
        const auto &call = static_cast<const CallExprNode &>(expr);
        out << "Call(" << call.callee << ")\n";
        for (const auto &arg : call.args) {
            printExpr(*arg, out, indent + 1);
        }
        break;
    }
    }
}

void printStmt(const StmtNode &stmt, std::ostream &out, int indent) {
    printIndent(out, indent);
    switch (stmt.kind) {
    case StmtKind::Assign: {
        const auto &assign = static_cast<const AssignStmtNode &>(stmt);
        out << "Assign(" << assign.target << ")\n";
        printExpr(*assign.value, out, indent + 1);
        break;
    }
    case StmtKind::Return: {
        const auto &ret = static_cast<const ReturnStmtNode &>(stmt);
        out << "Return\n";
        printExpr(*ret.value, out, indent + 1);
        break;
    }
    }
}

} // namespace

void printAst(const ModuleNode &module, std::ostream &out) {
    out << "Module\n";
    for (const auto &function : module.functions) {
        printIndent(out, 1);
        out << "Function " << function->name;
        if (!function->decorator.empty()) {
            out << " @" << function->decorator;
        }
        out << "\n";

        printIndent(out, 2);
        out << "Params\n";
        for (const auto &param : function->params) {
            printIndent(out, 3);
            out << param.name << ": ";
            printTensorType(param.type, out);
            out << "\n";
        }

        printIndent(out, 2);
        out << "Returns ";
        printTensorType(function->returnType, out);
        out << "\n";

        printIndent(out, 2);
        out << "Body\n";
        for (const auto &stmt : function->body) {
            printStmt(*stmt, out, 3);
        }
    }
}

} // namespace minitorch

