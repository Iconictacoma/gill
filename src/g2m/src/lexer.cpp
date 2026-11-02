#include "lexer.hpp"
#include <cctype>
namespace g2m {
Lexer::Lexer(const std::string& source) : source_(source) {}
bool Lexer::atEnd() const { return current_ >= source_.size(); }
char Lexer::peek() const { return atEnd() ? '\0' : source_[current_]; }
char Lexer::advance() { char c=source_[current_++]; if(c=='\n'){++line_;column_=1;}else ++column_; return c; }
bool Lexer::match(char expected){if(atEnd()||peek()!=expected)return false;advance();return true;}
Token Lexer::make(TokenKind k,std::size_t s,std::size_t l,std::size_t c)const{return{k,source_.substr(s,current_-s),l,c};}
void Lexer::skipWhitespaceAndComments(){for(;;){while(!atEnd()&&std::isspace(static_cast<unsigned char>(peek())))advance();if(peek()=='/'&&current_+1<source_.size()&&source_[current_+1]=='/'){while(!atEnd()&&peek()!='\n')advance();continue;}break;}}
Token Lexer::scanToken(){
 skipWhitespaceAndComments();auto start=current_, line=line_, col=column_;if(atEnd())return make(TokenKind::EndOfFile,start,line,col);char c=advance();
 if(std::isalpha(static_cast<unsigned char>(c))||c=='_'){
  while(std::isalnum(static_cast<unsigned char>(peek()))||peek()=='_')advance();auto t=make(TokenKind::Identifier,start,line,col);
  if(t.lexeme=="import")t.kind=TokenKind::KwImport;else if(t.lexeme=="module")t.kind=TokenKind::KwModule;else if(t.lexeme=="pub")t.kind=TokenKind::KwPub;else if(t.lexeme=="fn")t.kind=TokenKind::KwFn;else if(t.lexeme=="nil")t.kind=TokenKind::KwNil;
  else if(t.lexeme=="i32")t.kind=TokenKind::KwI32;else if(t.lexeme=="i64")t.kind=TokenKind::KwI64;else if(t.lexeme=="u32")t.kind=TokenKind::KwU32;else if(t.lexeme=="u64")t.kind=TokenKind::KwU64;else if(t.lexeme=="f32")t.kind=TokenKind::KwF32;else if(t.lexeme=="f64")t.kind=TokenKind::KwF64;else if(t.lexeme=="bool")t.kind=TokenKind::KwBool;else if(t.lexeme=="char")t.kind=TokenKind::KwChar;else if(t.lexeme=="string")t.kind=TokenKind::KwString;else if(t.lexeme=="mut")t.kind=TokenKind::KwMut;else if(t.lexeme=="const")t.kind=TokenKind::KwConst;else if(t.lexeme=="true")t.kind=TokenKind::KwTrue;else if(t.lexeme=="false")t.kind=TokenKind::KwFalse;return t;
 }
 if(std::isdigit(static_cast<unsigned char>(c))){while(std::isdigit(static_cast<unsigned char>(peek())))advance();if(peek()=='.'&&current_+1<source_.size()&&std::isdigit(static_cast<unsigned char>(source_[current_+1]))){advance();while(std::isdigit(static_cast<unsigned char>(peek())))advance();}return make(TokenKind::Number,start,line,col);}
 if(c=='"'){while(!atEnd()&&peek()!='"'){if(peek()=='\n'){errors_.push_back("line "+std::to_string(line)+":"+std::to_string(col)+": newline in string literal");return make(TokenKind::Invalid,start,line,col);}if(peek()=='\\'){advance();if(!atEnd())advance();}else advance();}if(atEnd()){errors_.push_back("line "+std::to_string(line)+":"+std::to_string(col)+": unterminated string literal");return make(TokenKind::Invalid,start,line,col);}advance();return make(TokenKind::String,start,line,col);}
 TokenKind k=TokenKind::Invalid;switch(c){case '(':k=TokenKind::LeftParen;break;case ')':k=TokenKind::RightParen;break;case '{':k=TokenKind::LeftBrace;break;case '}':k=TokenKind::RightBrace;break;case ';':k=TokenKind::Semicolon;break;case ',':k=TokenKind::Comma;break;case '=':k=TokenKind::Equal;break;case ':':if(match(':'))k=TokenKind::ColonColon;break;case '.':if(match('.')&&match('.'))k=TokenKind::Ellipsis;break;default:break;}
 if(k==TokenKind::Invalid)errors_.push_back("line "+std::to_string(line)+":"+std::to_string(col)+": unexpected character '"+c+"'");return make(k,start,line,col);
}
std::vector<Token> Lexer::scan(){std::vector<Token> v;do{v.push_back(scanToken());}while(v.back().kind!=TokenKind::EndOfFile);return v;}
}
