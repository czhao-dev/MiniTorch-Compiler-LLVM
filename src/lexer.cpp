#include "lexer.h"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace minitorch {

namespace {

bool isIdentStart(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool isIdentPart(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

} // namespace

const char *tokenTypeName(TokenType type) {
    switch (type) {
    case TokenType::Def:
        return "Def";
    case TokenType::Return:
        return "Return";
    case TokenType::If:
        return "If";
    case TokenType::Else:
        return "Else";
    case TokenType::At:
        return "At";
    case TokenType::Ident:
        return "Ident";
    case TokenType::Number:
        return "Number";
    case TokenType::Colon:
        return "Colon";
    case TokenType::Arrow:
        return "Arrow";
    case TokenType::LBracket:
        return "LBracket";
    case TokenType::RBracket:
        return "RBracket";
    case TokenType::LParen:
        return "LParen";
    case TokenType::RParen:
        return "RParen";
    case TokenType::Comma:
        return "Comma";
    case TokenType::Equal:
        return "Equal";
    case TokenType::Plus:
        return "Plus";
    case TokenType::Star:
        return "Star";
    case TokenType::Dot:
        return "Dot";
    case TokenType::Newline:
        return "Newline";
    case TokenType::Indent:
        return "Indent";
    case TokenType::Dedent:
        return "Dedent";
    case TokenType::EndOfFile:
        return "EndOfFile";
    }
    return "Unknown";
}

Lexer::Lexer(std::string source) : source_(std::move(source)), indentStack_{0} {}

std::vector<Token> Lexer::tokenize() {
    std::istringstream input(source_);
    std::string lineText;
    std::size_t lineNumber = 1;

    while (std::getline(input, lineText)) {
        if (!lineText.empty() && lineText.back() == '\r') {
            lineText.pop_back();
        }
        lexLine(lineText, lineNumber++);
    }

    while (indentStack_.size() > 1) {
        indentStack_.pop_back();
        add(TokenType::Dedent, "", lineNumber, 1);
    }
    add(TokenType::EndOfFile, "", lineNumber, 1);
    return tokens_;
}

void Lexer::lexLine(const std::string &lineText, std::size_t lineNumber) {
    std::size_t pos = 0;
    std::size_t indent = 0;
    while (pos < lineText.size() && (lineText[pos] == ' ' || lineText[pos] == '\t')) {
        indent += lineText[pos] == '\t' ? 4 : 1;
        ++pos;
    }

    if (pos == lineText.size() || lineText[pos] == '#') {
        return;
    }

    if (indent > indentStack_.back()) {
        indentStack_.push_back(indent);
        add(TokenType::Indent, "", lineNumber, 1);
    } else {
        while (indent < indentStack_.back()) {
            indentStack_.pop_back();
            add(TokenType::Dedent, "", lineNumber, 1);
        }
        if (indent != indentStack_.back()) {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": inconsistent indentation");
        }
    }

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"def", TokenType::Def},
        {"return", TokenType::Return},
        {"if", TokenType::If},
        {"else", TokenType::Else},
    };

    while (pos < lineText.size()) {
        const char ch = lineText[pos];
        const std::size_t column = pos + 1;

        if (ch == ' ' || ch == '\t') {
            ++pos;
            continue;
        }
        if (ch == '#') {
            break;
        }
        if (isIdentStart(ch)) {
            std::size_t start = pos++;
            while (pos < lineText.size() && isIdentPart(lineText[pos])) {
                ++pos;
            }
            std::string lexeme = lineText.substr(start, pos - start);
            auto found = keywords.find(lexeme);
            add(found == keywords.end() ? TokenType::Ident : found->second, lexeme, lineNumber, column);
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            std::size_t start = pos++;
            while (pos < lineText.size() && std::isdigit(static_cast<unsigned char>(lineText[pos]))) {
                ++pos;
            }
            if (pos < lineText.size() && lineText[pos] == '.') {
                ++pos;
                while (pos < lineText.size() && std::isdigit(static_cast<unsigned char>(lineText[pos]))) {
                    ++pos;
                }
            }
            add(TokenType::Number, lineText.substr(start, pos - start), lineNumber, column);
            continue;
        }

        if (ch == '-' && pos + 1 < lineText.size() && lineText[pos + 1] == '>') {
            add(TokenType::Arrow, "->", lineNumber, column);
            pos += 2;
            continue;
        }

        switch (ch) {
        case '@':
            add(TokenType::At, "@", lineNumber, column);
            break;
        case ':':
            add(TokenType::Colon, ":", lineNumber, column);
            break;
        case '[':
            add(TokenType::LBracket, "[", lineNumber, column);
            break;
        case ']':
            add(TokenType::RBracket, "]", lineNumber, column);
            break;
        case '(':
            add(TokenType::LParen, "(", lineNumber, column);
            break;
        case ')':
            add(TokenType::RParen, ")", lineNumber, column);
            break;
        case ',':
            add(TokenType::Comma, ",", lineNumber, column);
            break;
        case '=':
            add(TokenType::Equal, "=", lineNumber, column);
            break;
        case '+':
            add(TokenType::Plus, "+", lineNumber, column);
            break;
        case '*':
            add(TokenType::Star, "*", lineNumber, column);
            break;
        case '.':
            add(TokenType::Dot, ".", lineNumber, column);
            break;
        default:
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": unexpected character '" + ch + "'");
        }
        ++pos;
    }

    add(TokenType::Newline, "", lineNumber, lineText.size() + 1);
}

void Lexer::add(TokenType type, std::string lexeme, std::size_t line, std::size_t column) {
    tokens_.push_back(Token{type, std::move(lexeme), line, column});
}

} // namespace minitorch

