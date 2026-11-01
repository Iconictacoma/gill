#pragma once
#include <string>
#include <vector>

namespace g2m {
struct Statement {
    enum class Kind { PrintString, PrintIdentifier, VariableI32 };
    Kind kind;
    std::string name;
    std::string value;
};

struct Function {
    std::string name;
    std::string returnType;
    std::vector<Statement> body;
};

struct Program {
    std::vector<std::string> imports;
    std::vector<Function> functions;
};
}
