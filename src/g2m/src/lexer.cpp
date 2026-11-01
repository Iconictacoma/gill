#include "lexer.hpp"
#include <cctype>
#include <sstream>

namespace g2m {
Lexer::Lexer(const std::string& source) : source_(source) {}

bool Lexer::atEnd() const { return current_ >= source_.size(); }
char Lexer::peek() const { return atEnd() ? '\0' : source_[current_]; }

char Lexer::advance() {
    char c = source_[current_++];
    if (c == '\n') { ++line_; column_ = 1; }
    else { ++column_; }
    return c;
}

bool Lexer::match(char expected) {
    if (atEnd() || peek() != expected) return false;
    advance();
    return true;
}

Token Lexer::make(TokenKind kind, std::size_t start, std::size_t line,
                  std::size_t column) const {
    return {kind, source_.substr(start, current_ - start), line, column};
}

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        while (!atEnd() && std::isspace(static_cast<unsigned char>(peek())))
            advance();

        if (peek() == '/' && current_ + 1 < source_.size()
            && source_[current_ + 1] == '/') {
            while (!atEnd() && peek() != '\n') advance();
            continue;
        }
        break;
    }
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();
    const auto start = current_;
    const auto startLine = line_;
    const auto startColumn = column_;
    if (atEnd()) return make(TokenKind::EndOfFile, start, startLine, startColumn);

    char c = advance();
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
            advance();
        Token token = make(TokenKind::Identifier, start, startLine, startColumn);
        if (token.lexeme == "import") token.kind = TokenKind::KwImport;
        else if (token.lexeme == "fn") token.kind = TokenKind::KwFn;
        else if (token.lexeme == "nil") token.kind = TokenKind::KwNil;
        else if (token.lexeme == "i32") token.kind = TokenKind::KwI32;
        else if (token.lexeme == "mut") token.kind = TokenKind::KwMut;
        else if (token.lexeme == "const") token.kind = TokenKind::KwConst;
        return token;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
        return make(TokenKind::Number, start, startLine, startColumn);
    }

    if (c == '"') {
        while (!atEnd() && peek() != '"') {
            if (peek() == '\n') {
                errors_.push_back("line " + std::to_string(startLine) +
                                  ":" + std::to_string(startColumn) +
                                  ": newline in string literal");
                return make(TokenKind::Invalid, start, startLine, startColumn);
            }
            if (peek() == '\\') {
                advance();
                if (!atEnd()) advance();
            } else advance();
        }
        if (atEnd()) {
            errors_.push_back("line " + std::to_string(startLine) +
                              ":" + std::to_string(startColumn) +
                              ": unterminated string literal");
            return make(TokenKind::Invalid, start, startLine, startColumn);
        }
        advance();
        return make(TokenKind::String, start, startLine, startColumn);
    }

    TokenKind kind = TokenKind::Invalid;
    switch (c) {
        case '(': kind = TokenKind::LeftParen; break;
        case ')': kind = TokenKind::RightParen; break;
        case '{': kind = TokenKind::LeftBrace; break;
        case '}': kind = TokenKind::RightBrace; break;
        case ';': kind = TokenKind::Semicolon; break;
        case ',': kind = TokenKind::Comma; break;
        case '=': kind = TokenKind::Equal; break;
        case ':':
            if (match(':')) kind = TokenKind::ColonColon;
            else errors_.push_back("line " + std::to_string(startLine) + ":" +
                std::to_string(startColumn) + ": expected second ':' in module path");
            break;
        default:
            errors_.push_back("line " + std::to_string(startLine) + ":" +
                std::to_string(startColumn) + ": unexpected character '" + c + "'");
    }
    return make(kind, start, startLine, startColumn);
}

std::vector<Token> Lexer::scan() {
    std::vector<Token> tokens;
    do { tokens.push_back(scanToken()); }
    while (tokens.back().kind != TokenKind::EndOfFile);
    return tokens;
}
}
