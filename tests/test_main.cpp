#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "typechecker.h"

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<minitorch::Token> lex(const std::string &source) {
    minitorch::Lexer lexer(source);
    return lexer.tokenize();
}

void testLexerEmitsIndentDedent() {
    const std::string source =
        "@minitorch.compile\n"
        "def linear(x: Tensor[2, 3]) -> Tensor[2, 3]:\n"
        "    y = relu(x)\n"
        "    return y\n";

    const auto tokens = lex(source);
    bool sawIndent = false;
    bool sawDedent = false;
    for (const auto &token : tokens) {
        sawIndent = sawIndent || token.type == minitorch::TokenType::Indent;
        sawDedent = sawDedent || token.type == minitorch::TokenType::Dedent;
    }

    assert(sawIndent);
    assert(sawDedent);
}

void testParserParsesFunction() {
    const std::string source =
        "@minitorch.compile\n"
        "def linear(x: Tensor[2, 3], w: Tensor[3, 4]) -> Tensor[2, 4]:\n"
        "    y = matmul(x, w)\n"
        "    return y\n";

    minitorch::Parser parser(lex(source));
    minitorch::ModuleNode module = parser.parseModule();
    minitorch::TypeChecker checker;
    checker.check(module);

    assert(module.functions.size() == 1);
    assert(module.functions[0]->decorator == "minitorch.compile");
    assert(module.functions[0]->name == "linear");
    assert(module.functions[0]->params.size() == 2);
    assert(module.functions[0]->returnType.dims.size() == 2);
    assert(module.functions[0]->body.size() == 2);

    std::ostringstream out;
    minitorch::printAst(module, out);
    assert(out.str().find("Function linear") != std::string::npos);
    assert(out.str().find("Call(matmul)") != std::string::npos);
}

void testOperatorPrecedence() {
    const std::string source =
        "@minitorch.compile\n"
        "def f(x: Tensor[1]) -> Tensor[1]:\n"
        "    y = a + b * c\n"
        "    return y\n";

    minitorch::Parser parser(lex(source));
    minitorch::ModuleNode module = parser.parseModule();
    const auto &assign = static_cast<const minitorch::AssignStmtNode &>(*module.functions[0]->body[0]);
    const auto &expr = static_cast<const minitorch::BinOpExprNode &>(*assign.value);
    assert(expr.op == minitorch::BinaryOp::Add);
    assert(expr.right->kind == minitorch::ExprKind::Binary);
}

} // namespace

int main() {
    testLexerEmitsIndentDedent();
    testParserParsesFunction();
    testOperatorPrecedence();
    return 0;
}

