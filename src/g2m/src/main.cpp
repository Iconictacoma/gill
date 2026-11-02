#include "ir.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>
int main(int argc,char*argv[]){if(argc<2){std::cerr<<"Usage: g2m <file.gill> [-c output] [-Ro]\n";return 1;}const std::string input=argv[1];std::ifstream file(input);if(!file){std::cerr<<"g2m: cannot open '"<<input<<"'\n";return 1;}const std::string source((std::istreambuf_iterator<char>(file)),{});g2m::Lexer lexer(source);auto tokens=lexer.scan();for(const auto&e:lexer.errors())std::cerr<<"g2m: "<<e<<'\n';if(!lexer.errors().empty())return 1;g2m::Parser parser(tokens);auto p=parser.parse();for(const auto&e:parser.errors())std::cerr<<"g2m: "<<e<<'\n';if(!parser.errors().empty())return 1;
 bool hasIO=false;for(const auto&i:p.imports)if(i=="stdlib"||i=="stdlib::io")hasIO=true;for(const auto&f:p.functions)for(const auto&s:f.body)if((s.kind==g2m::Statement::Kind::PrintString||s.kind==g2m::Statement::Kind::Printf)&&!hasIO){std::cerr<<"g2m: output functions require 'import stdlib;' or 'import stdlib::io;'\n";return 1;}
 std::unordered_set<std::string> functionNames;for(const auto&f:p.functions){if(!functionNames.insert(f.name).second){std::cerr<<"g2m: duplicate function '"<<f.name<<"'\n";return 1;}for(std::size_t i=0;i<f.parameters.size();++i)if(f.parameters[i].variadic&&i+1!=f.parameters.size()){std::cerr<<"g2m: variadic parameter must be final in '"<<f.name<<"'\n";return 1;}}
 auto ir=g2m::lowerToIR(p);std::cout<<"g2m Gill IR prototype\n"<<printIR(ir);std::cout<<"Note: this build parses module declarations and typed variadic signatures; it emits prototype IR, not machine code.\n";return 0;}
