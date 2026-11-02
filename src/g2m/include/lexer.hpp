#pragma once
#include "token.hpp"
#include <string>
#include <vector>
namespace g2m {
class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> scan();
    const std::vector<std::string>& errors() const { return errors_; }
private:
    const std::string& source_;
    std::size_t current_ = 0, line_ = 1, column_ = 1;
    std::vector<std::string> errors_;
    bool atEnd() const; char peek() const; char advance(); bool match(char expected);
    void skipWhitespaceAndComments(); Token scanToken();
    Token make(TokenKind kind, std::size_t start, std::size_t line, std::size_t column) const;
};
}
