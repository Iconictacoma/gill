#pragma once
#include "ast.hpp"
#include "token.hpp"
#include <string>
#include <vector>

namespace g2m {
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    Program parse();
    const std::vector<std::string>& errors() const { return errors_; }

private:
    const std::vector<Token>& tokens_;
    std::size_t current_ = 0;
    std::vector<std::string> errors_;

    const Token& peek() const;
    const Token& previous() const;
    bool check(TokenKind kind) const;
    bool match(TokenKind kind);
    const Token& consume(TokenKind kind, const std::string& message);
    void error(const Token& token, const std::string& message);
    std::string parseModulePath();
    Function parseFunction();
    Statement parseStatement();
    void synchronize();
};
}
