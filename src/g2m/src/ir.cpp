#include "ir.hpp"
#include <sstream>

namespace g2m {
IRModule lowerToIR(const Program& program) {
    IRModule module;
    for (const auto& function : program.functions) {
        IRFunction out{function.name, {}};
        int nextValue = 0;
        for (const auto& statement : function.body) {
            switch (statement.kind) {
                case Statement::Kind::VariableI32:
                    out.instructions.push_back({"const.i32", "%v" +
                        std::to_string(nextValue++), statement.value});
                    out.instructions.push_back({"local.set", statement.name,
                        out.instructions.back().result});
                    break;
                case Statement::Kind::PrintString:
                    out.instructions.push_back({"call", "printn",
                        statement.value});
                    break;
                case Statement::Kind::PrintIdentifier:
                    out.instructions.push_back({"call", "printn",
                        statement.name});
                    break;
            }
        }
        out.instructions.push_back({"return", "", "nil"});
        module.functions.push_back(std::move(out));
    }
    return module;
}

std::string printIR(const IRModule& module) {
    std::ostringstream out;
    for (const auto& function : module.functions) {
        out << "function @" << function.name << " {\n";
        for (const auto& i : function.instructions) {
            out << "  ";
            if (!i.result.empty()) out << i.result << " = ";
            out << i.opcode;
            if (!i.operand.empty()) out << " " << i.operand;
            out << "\n";
        }
        out << "}\n";
    }
    return out.str();
}
}
