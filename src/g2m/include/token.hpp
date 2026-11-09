#pragma once
#include <cstddef>
#include <string>
namespace g2m {
enum class TokenKind {
 Identifier, Number, String, Character,
 KwImport,KwModule,KwPub,KwFn,KwNil,KwReturn,KwI8,KwI16,KwI32,KwI64,KwI128,KwU8,KwU16,KwU32,KwU64,KwU128,KwIsize,KwUsize,KwInt,KwF32,KwF64,KwBool,KwChar,KwString,KwMut,KwConst,KwTrue,KwFalse,
 KwIf,KwElse,KwWhile,KwFor,KwBreak,KwContinue,
 LeftParen,RightParen,LeftBrace,RightBrace,LeftBracket,RightBracket,
 Semicolon,Comma,Equal,Plus,Minus,Star,Slash,Percent,Bang,
 EqualEqual,BangEqual,Less,LessEqual,Greater,GreaterEqual,
 AndAnd,OrOr,PlusPlus,MinusMinus,PlusEqual,MinusEqual,
 ColonColon,Ellipsis,EndOfFile,Invalid
};
struct Token {TokenKind kind;std::string lexeme;std::size_t line;std::size_t column;};
}