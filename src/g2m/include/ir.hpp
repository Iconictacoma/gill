#pragma once
#include "ast.hpp"
#include <string>
#include <vector>
namespace g2m {
struct IRInstruction{std::string opcode,result,operand;};struct IRFunction{std::string name;std::vector<IRInstruction> instructions;};struct IRModule{std::string name;std::vector<IRFunction> functions;};
IRModule lowerToIR(const Program&);std::string printIR(const IRModule&);
}
