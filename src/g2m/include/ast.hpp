#pragma once
#include <string>
#include <vector>
#include <memory>
namespace g2m {
struct Parameter {std::string type,name;bool variadic=false;};
struct Statement {
 enum class Kind {Variable,Expression,Return,If,While,For,Block,Break,Continue};
 Kind kind=Kind::Expression; std::string name,value,declaredType,condition,increment;
 bool mutableValue=false,constant=false;
 std::vector<std::string> arguments;
 std::vector<Statement> body,otherwise;
};
struct Function {std::string name,returnType;std::vector<Parameter> parameters;std::vector<Statement> body;bool declarationOnly=false;};
struct Program {std::string moduleName;bool modulePublic=false;std::vector<std::string> imports;std::vector<Function> functions;};
}