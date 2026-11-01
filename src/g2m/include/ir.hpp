#pragma once
#include "ast.hpp"
#include <string>
#include <vector>

namespace g2m {
struct IRInstruction {
    std::string opcode;
    std::string result;
    std::string operand;
};
struct IRFunction {
    std::string name;
    std::vector<IRInstruction> instructions;
};
struct IRModule {
    std::vector<IRFunction> functions;
};
IRModule lowerToIR(const Program& program);
std::string printIR(const IRModule& module);
}
