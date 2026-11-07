#pragma once
#include <cstddef>
#include <string>

namespace g2m {
enum class TokenKind {
    Identifier, Number, String,
    KwImport, KwModule, KwPub, KwFn, KwNil, KwI8, KwI16, KwI32, KwI64, KwI128, KwU8, KwU16, KwU32, KwU64, KwU128, KwIsize, KwUsize, KwInt,
    KwF32, KwF64, KwBool, KwChar, KwString, KwMut, KwConst, KwTrue, KwFalse,
    LeftParen, RightParen, LeftBrace, RightBrace,
    Semicolon, Comma, Equal, ColonColon, Ellipsis,
    EndOfFile, Invalid
};
struct Token { TokenKind kind; std::string lexeme; std::size_t line; std::size_t column; };
}
