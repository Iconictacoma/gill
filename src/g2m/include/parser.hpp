#pragma once
#include "ast.hpp"
#include "token.hpp"
#include <string>
#include <vector>
namespace g2m {
class Parser {
public:
 explicit Parser(const std::vector<Token>& tokens); Program parse();
 const std::vector<std::string>& errors() const{return errors_;}
private:
 const std::vector<Token>& tokens_;std::size_t current_=0;std::vector<std::string> errors_;
 const Token& peek()const;const Token& previous()const;bool check(TokenKind)const;bool match(TokenKind);const Token& consume(TokenKind,const std::string&);void error(const Token&,const std::string&);void synchronize();
 std::string parseModulePath();std::string parseType();Function parseFunction();Statement parseStatement();Argument parseArgument();
};
}
