#include "parser.hpp"
#include <sstream>

namespace g2m {
Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}
const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::check(TokenKind k) const { return peek().kind == k; }
bool Parser::match(TokenKind k) {
    if (!check(k)) return false;
    ++current_; return true;
}
const Token& Parser::consume(TokenKind k, const std::string& message) {
    if (check(k)) return tokens_[current_++];
    error(peek(), message);
    return peek();
}
void Parser::error(const Token& t, const std::string& message) {
    errors_.push_back("line " + std::to_string(t.line) + ":" +
        std::to_string(t.column) + ": " + message);
}
void Parser::synchronize() {
    while (!check(TokenKind::EndOfFile)) {
        if (previous().kind == TokenKind::Semicolon) return;
        if (check(TokenKind::KwFn) || check(TokenKind::KwImport)) return;
        ++current_;
    }
}

std::string Parser::parseModulePath() {
    std::string path;
    const Token& first = consume(TokenKind::Identifier, "expected module name");
    path = first.lexeme;
    while (match(TokenKind::ColonColon)) {
        const Token& part = consume(TokenKind::Identifier,
                                    "expected module name after '::'");
        path += "::" + part.lexeme;
    }
    return path;
}

Function Parser::parseFunction() {
    consume(TokenKind::KwFn, "expected 'fn'");
    const Token& name = consume(TokenKind::Identifier, "expected function name");
    consume(TokenKind::LeftParen, "expected '(' after function name");
    consume(TokenKind::RightParen, "this first compiler version supports no parameters");
    std::string returnType;
    if (match(TokenKind::KwNil)) returnType = "nil";
    else {
        error(peek(), "expected return type 'nil' in this compiler milestone");
        returnType = "nil";
    }
    consume(TokenKind::LeftBrace, "expected '{' before function body");
    Function function{name.lexeme, returnType, {}};
    while (!check(TokenKind::RightBrace) && !check(TokenKind::EndOfFile)) {
        try { function.body.push_back(parseStatement()); }
        catch (...) { synchronize(); }
    }
    consume(TokenKind::RightBrace, "expected '}' after function body");
    return function;
}

Statement Parser::parseStatement() {
    if (match(TokenKind::KwI32)) {
        const Token& name = consume(TokenKind::Identifier, "expected variable name");
        consume(TokenKind::Equal, "expected '=' in variable declaration");
        const Token& value = consume(TokenKind::Number, "expected integer literal");
        consume(TokenKind::Semicolon, "expected ';' after declaration");
        return {Statement::Kind::VariableI32, name.lexeme, value.lexeme};
    }
    consume(TokenKind::Identifier, "expected statement");
    const Token callee = previous();
    if (callee.lexeme != "printn") error(callee, "only 'printn' calls are supported so far");
    consume(TokenKind::LeftParen, "expected '(' after function name");
    Statement result{};
    if (match(TokenKind::String)) {
        result = {Statement::Kind::PrintString, "", previous().lexeme};
    } else if (match(TokenKind::Identifier)) {
        result = {Statement::Kind::PrintIdentifier, previous().lexeme, ""};
    } else {
        error(peek(), "expected string literal or identifier in printn call");
    }
    consume(TokenKind::RightParen, "expected ')' after argument");
    consume(TokenKind::Semicolon, "expected ';' after function call");
    return result;
}

Program Parser::parse() {
    Program program;
    while (!check(TokenKind::EndOfFile)) {
        if (match(TokenKind::KwImport)) {
            std::string path;
            if (check(TokenKind::Identifier)) path = parseModulePath();
            else error(peek(), "expected module name after import");
            consume(TokenKind::Semicolon, "expected ';' after import");
            program.imports.push_back(path);
        } else if (check(TokenKind::KwFn)) {
            program.functions.push_back(parseFunction());
        } else {
            error(peek(), "expected import or function declaration");
            ++current_;
        }
    }
    return program;
}
}
