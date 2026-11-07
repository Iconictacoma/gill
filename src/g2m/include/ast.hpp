#pragma once
#include <string>
#include <vector>

namespace g2m {
struct Parameter {
    std::string type;
    std::string name;
    bool variadic = false;
};
struct Argument {
    std::string type;
    std::string value;
    bool stringLiteral = false;
};
struct Statement {
    enum class Kind { PrintString, PrintIdentifier, Printf, VariableI32, Call, Return };
    Kind kind = Kind::Call;
    std::string name;
    std::string value;
    std::string declaredType;
    bool mutableValue = false;
    bool constant = false;
    std::vector<Argument> arguments;
};
struct Function {
    std::string name;
    std::string returnType;
    std::vector<Parameter> parameters;
    std::vector<Statement> body;
    bool declarationOnly = false;
};
struct Program {
    std::string moduleName;
    bool modulePublic = false;
    std::vector<std::string> imports;
    std::vector<Function> functions;
};
}
