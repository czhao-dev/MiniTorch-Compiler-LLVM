#include "parser.h"

#include <sstream>

namespace minitorch {

ParseError::ParseError(const Token &token, const std::string &message)
    : std::runtime_error("line " + std::to_string(token.line) + ", column " +
                         std::to_string(token.column) + ": " + message) {}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

ModuleNode Parser::parseModule() {
    ModuleNode module;
    while (!isAtEnd()) {
        while (match(TokenType::Newline)) {
        }
        if (isAtEnd()) {
            break;
        }
        module.functions.push_back(parseFuncDef());
    }
    return module;
}

std::unique_ptr<FuncDefNode> Parser::parseFuncDef() {
    std::string decorator;
    if (match(TokenType::At)) {
        decorator = parseDottedName();
        consume(TokenType::Newline, "expected newline after decorator");
    }

    const Token &defToken = consume(TokenType::Def, "expected function definition");
    const Token &name = consume(TokenType::Ident, "expected function name");
    consume(TokenType::LParen, "expected '(' after function name");

    std::vector<ParamNode> params;
    if (!check(TokenType::RParen)) {
        do {
            const Token &paramName = consume(TokenType::Ident, "expected parameter name");
            consume(TokenType::Colon, "expected ':' after parameter name");
            params.push_back(ParamNode{paramName.lexeme, parseTensorType()});
        } while (match(TokenType::Comma));
    }

    consume(TokenType::RParen, "expected ')' after parameter list");
    consume(TokenType::Arrow, "expected return type annotation");
    TensorTypeNode returnType = parseTensorType();
    consume(TokenType::Colon, "expected ':' after return type");
    consume(TokenType::Newline, "expected newline before function body");
    consume(TokenType::Indent, "expected indented function body");

    std::vector<std::unique_ptr<StmtNode>> body;
    while (!check(TokenType::Dedent) && !isAtEnd()) {
        if (match(TokenType::Newline)) {
            continue;
        }
        body.push_back(parseStatement());
    }
    consume(TokenType::Dedent, "expected dedent after function body");

    auto function = std::make_unique<FuncDefNode>();
    function->decorator = std::move(decorator);
    function->name = name.lexeme;
    function->params = std::move(params);
    function->returnType = std::move(returnType);
    function->body = std::move(body);
    function->line = defToken.line;
    return function;
}

std::unique_ptr<StmtNode> Parser::parseStatement() {
    if (match(TokenType::Return)) {
        auto value = parseExpression();
        consume(TokenType::Newline, "expected newline after return statement");
        return std::make_unique<ReturnStmtNode>(std::move(value));
    }

    const Token &target = consume(TokenType::Ident, "expected assignment target or return");
    consume(TokenType::Equal, "expected '=' after assignment target");
    auto value = parseExpression();
    consume(TokenType::Newline, "expected newline after assignment");
    return std::make_unique<AssignStmtNode>(target.lexeme, std::move(value));
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    return parseAdditive();
}

std::unique_ptr<ExprNode> Parser::parseAdditive() {
    auto expr = parseMultiplicative();
    while (match(TokenType::Plus)) {
        auto right = parseMultiplicative();
        expr = std::make_unique<BinOpExprNode>(BinaryOp::Add, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseMultiplicative() {
    auto expr = parsePrimary();
    while (match(TokenType::Star)) {
        auto right = parsePrimary();
        expr = std::make_unique<BinOpExprNode>(BinaryOp::Mul, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    if (match(TokenType::Number)) {
        return std::make_unique<NumberExprNode>(std::stod(previous().lexeme));
    }

    if (match(TokenType::Ident)) {
        std::string name = previous().lexeme;
        while (match(TokenType::Dot)) {
            const Token &part = consume(TokenType::Ident, "expected identifier after '.'");
            name += "." + part.lexeme;
        }

        if (match(TokenType::LParen)) {
            std::vector<std::unique_ptr<ExprNode>> args;
            if (!check(TokenType::RParen)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::Comma));
            }
            consume(TokenType::RParen, "expected ')' after call arguments");
            return std::make_unique<CallExprNode>(std::move(name), std::move(args));
        }
        return std::make_unique<IdentExprNode>(std::move(name));
    }

    if (match(TokenType::LParen)) {
        auto expr = parseExpression();
        consume(TokenType::RParen, "expected ')' after expression");
        return expr;
    }

    throw ParseError(peek(), "expected expression");
}

TensorTypeNode Parser::parseTensorType() {
    const Token &name = consume(TokenType::Ident, "expected type name");
    if (name.lexeme != "Tensor") {
        throw ParseError(name, "expected Tensor type annotation");
    }

    consume(TokenType::LBracket, "expected '[' after Tensor");
    TensorTypeNode type;
    do {
        const Token &dim = consume(TokenType::Number, "expected tensor dimension");
        type.dims.push_back(std::stoi(dim.lexeme));
    } while (match(TokenType::Comma));
    consume(TokenType::RBracket, "expected ']' after tensor dimensions");
    return type;
}

std::string Parser::parseDottedName() {
    const Token &first = consume(TokenType::Ident, "expected decorator name");
    std::string name = first.lexeme;
    while (match(TokenType::Dot)) {
        const Token &part = consume(TokenType::Ident, "expected identifier after '.'");
        name += "." + part.lexeme;
    }
    return name;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return type == TokenType::EndOfFile;
    }
    return peek().type == type;
}

const Token &Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

const Token &Parser::consume(TokenType type, const std::string &message) {
    if (check(type)) {
        return advance();
    }
    throw ParseError(peek(), message);
}

const Token &Parser::peek() const {
    return tokens_[current_];
}

const Token &Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return current_ >= tokens_.size() || tokens_[current_].type == TokenType::EndOfFile;
}

} // namespace minitorch

