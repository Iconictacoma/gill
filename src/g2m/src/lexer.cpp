#include "lexer.hpp"
#include <cctype>
#include <unordered_map>
namespace g2m {
Lexer::Lexer(const std::string&s):source_(s){}
bool Lexer::atEnd()const{return current_>=source_.size();}
char Lexer::peek()const{return atEnd()?'\0':source_[current_];}
char Lexer::advance(){char c=source_[current_++];if(c=='\n'){++line_;column_=1;}else ++column_;return c;}
bool Lexer::match(char c){if(atEnd()||peek()!=c)return false;advance();return true;}
Token Lexer::make(TokenKind k,std::size_t s,std::size_t l,std::size_t c)const{return{k,source_.substr(s,current_-s),l,c};}
void Lexer::skipWhitespaceAndComments(){for(;;){while(!atEnd()&&std::isspace((unsigned char)peek()))advance();if(peek()=='/'&&current_+1<source_.size()&&source_[current_+1]=='/'){while(!atEnd()&&peek()!='\n')advance();}else if(peek()=='/'&&current_+1<source_.size()&&source_[current_+1]=='*'){advance();advance();while(!atEnd()&&!(peek()=='*'&&current_+1<source_.size()&&source_[current_+1]=='/'))advance();if(!atEnd()){advance();advance();}}else break;}}
Token Lexer::scanToken(){skipWhitespaceAndComments();auto s=current_,l=line_,c=column_;if(atEnd())return make(TokenKind::EndOfFile,s,l,c);char ch=advance();
 if(std::isalpha((unsigned char)ch)||ch=='_'){while(std::isalnum((unsigned char)peek())||peek()=='_')advance();auto t=make(TokenKind::Identifier,s,l,c);
 static const std::unordered_map<std::string,TokenKind> kw={{"import",TokenKind::KwImport},{"module",TokenKind::KwModule},{"pub",TokenKind::KwPub},{"fn",TokenKind::KwFn},{"nil",TokenKind::KwNil},{"return",TokenKind::KwReturn},{"i8",TokenKind::KwI8},{"i16",TokenKind::KwI16},{"i32",TokenKind::KwI32},{"i64",TokenKind::KwI64},{"i128",TokenKind::KwI128},{"u8",TokenKind::KwU8},{"u16",TokenKind::KwU16},{"u32",TokenKind::KwU32},{"u64",TokenKind::KwU64},{"u128",TokenKind::KwU128},{"isize",TokenKind::KwIsize},{"usize",TokenKind::KwUsize},{"int",TokenKind::KwInt},{"f32",TokenKind::KwF32},{"f64",TokenKind::KwF64},{"bool",TokenKind::KwBool},{"char",TokenKind::KwChar},{"string",TokenKind::KwString},{"mut",TokenKind::KwMut},{"const",TokenKind::KwConst},{"true",TokenKind::KwTrue},{"false",TokenKind::KwFalse},{"if",TokenKind::KwIf},{"else",TokenKind::KwElse},{"while",TokenKind::KwWhile},{"for",TokenKind::KwFor},{"break",TokenKind::KwBreak},{"continue",TokenKind::KwContinue}};
 auto it=kw.find(t.lexeme);if(it!=kw.end())t.kind=it->second;return t;}
 if(std::isdigit((unsigned char)ch)){while(std::isdigit((unsigned char)peek()))advance();if(peek()=='.'&&current_+1<source_.size()&&std::isdigit((unsigned char)source_[current_+1])){advance();while(std::isdigit((unsigned char)peek()))advance();}return make(TokenKind::Number,s,l,c);}
 if(ch=='"'||ch=='\''){char end=ch;while(!atEnd()&&peek()!=end){if(peek()=='\\'){advance();if(!atEnd())advance();}else advance();}if(atEnd()){errors_.push_back("line "+std::to_string(l)+": unterminated literal");return make(TokenKind::Invalid,s,l,c);}advance();return make(end=='"'?TokenKind::String:TokenKind::Character,s,l,c);}
 TokenKind k=TokenKind::Invalid;switch(ch){
 case '(':k=TokenKind::LeftParen;break;case ')':k=TokenKind::RightParen;break;case '{':k=TokenKind::LeftBrace;break;case '}':k=TokenKind::RightBrace;break;case '[':k=TokenKind::LeftBracket;break;case ']':k=TokenKind::RightBracket;break;case ';':k=TokenKind::Semicolon;break;case ',':k=TokenKind::Comma;break;
 case ':':if(match(':'))k=TokenKind::ColonColon;break;case '.':if(match('.')&&match('.'))k=TokenKind::Ellipsis;break;
 case '+':k=match('+')?TokenKind::PlusPlus:match('=')?TokenKind::PlusEqual:TokenKind::Plus;break;case '-':k=match('-')?TokenKind::MinusMinus:match('=')?TokenKind::MinusEqual:TokenKind::Minus;break;
 case '*':k=TokenKind::Star;break;case '/':k=TokenKind::Slash;break;case '%':k=TokenKind::Percent;break;
 case '=':k=match('=')?TokenKind::EqualEqual:TokenKind::Equal;break;case '!':k=match('=')?TokenKind::BangEqual:TokenKind::Bang;break;
 case '<':k=match('=')?TokenKind::LessEqual:TokenKind::Less;break;case '>':k=match('=')?TokenKind::GreaterEqual:TokenKind::Greater;break;
 case '&':if(match('&'))k=TokenKind::AndAnd;break;case '|':if(match('|'))k=TokenKind::OrOr;break;
 }
 if(k==TokenKind::Invalid)errors_.push_back("line "+std::to_string(l)+":"+std::to_string(c)+": unexpected character");return make(k,s,l,c);
}
std::vector<Token> Lexer::scan(){std::vector<Token>v;do{v.push_back(scanToken());}while(v.back().kind!=TokenKind::EndOfFile);return v;}
}