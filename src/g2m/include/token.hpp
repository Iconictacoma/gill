#pragma once
#include <cstddef>
#include <string>

namespace g2m {
enum class TokenKind {
    Identifier, Number, String,
    KwImport, KwFn, KwNil, KwI32, KwMut, KwConst,
    LeftParen, RightParen, LeftBrace, RightBrace,
    Semicolon, Comma, Equal, ColonColon,
    EndOfFile, Invalid
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};
}
