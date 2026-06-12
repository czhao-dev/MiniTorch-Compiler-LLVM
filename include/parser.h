#pragma once

#include "ast.h"
#include "lexer.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace minitorch {

class ParseError final : public std::runtime_error {
public:
    ParseError(const Token &token, const std::string &message);
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    ModuleNode parseModule();

private:
    std::vector<Token> tokens_;
    std::size_t current_ = 0;

    std::unique_ptr<FuncDefNode> parseFuncDef();
    std::unique_ptr<StmtNode> parseStatement();
    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseAdditive();
    std::unique_ptr<ExprNode> parseMultiplicative();
    std::unique_ptr<ExprNode> parsePrimary();
    TensorTypeNode parseTensorType();
    std::string parseDottedName();

    bool match(TokenType type);
    bool check(TokenType type) const;
    const Token &advance();
    const Token &consume(TokenType type, const std::string &message);
    const Token &peek() const;
    const Token &previous() const;
    bool isAtEnd() const;
};

} // namespace minitorch

