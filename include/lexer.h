#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace minitorch {

enum class TokenType {
    Def,
    Return,
    If,
    Else,
    At,
    Ident,
    Number,
    Colon,
    Arrow,
    LBracket,
    RBracket,
    LParen,
    RParen,
    Comma,
    Equal,
    Plus,
    Star,
    Dot,
    Newline,
    Indent,
    Dedent,
    EndOfFile,
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};

const char *tokenTypeName(TokenType type);

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    std::vector<Token> tokens_;
    std::vector<std::size_t> indentStack_;

    void lexLine(const std::string &lineText, std::size_t lineNumber);
    void add(TokenType type, std::string lexeme, std::size_t line, std::size_t column);
};

} // namespace minitorch

