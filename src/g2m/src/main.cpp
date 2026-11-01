#include "ir.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: g2m <file.gill> [-c output] [-Ro]\n note: -Ro for release optimization.";
        return 1;
    }
    const std::string inputPath = argv[1];
    std::ifstream file(inputPath);
    if (!file) {
        std::cerr << "g2m: cannot open '" << inputPath << "'\n";
        return 1;
    }
    const std::string source((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

    g2m::Lexer lexer(source);
    auto tokens = lexer.scan();
    for (const auto& e : lexer.errors()) std::cerr << "g2m: " << e << '\n';
    if (!lexer.errors().empty()) return 1;

    g2m::Parser parser(tokens);
    auto program = parser.parse();
    for (const auto& e : parser.errors()) std::cerr << "g2m: " << e << '\n';
    if (!parser.errors().empty()) return 1;

    bool hasStdlib = false;
    for (const auto& import : program.imports)
        if (import == "stdlib") hasStdlib = true;

    for (const auto& function : program.functions) {
        for (const auto& statement : function.body) {
            if ((statement.kind == g2m::Statement::Kind::PrintString ||
                 statement.kind == g2m::Statement::Kind::PrintIdentifier) &&
                !hasStdlib) {
                std::cerr << "g2m: printn requires 'import stdlib;' in this "
                             "compiler version\n";
                return 1;
            }
        }
    }

    const auto ir = g2m::lowerToIR(program);
    std::cout << "g2m PROTOTYPE\n";
    std::cout << "Parsed " << program.functions.size() << " function(s).\n";
    std::cout << printIR(ir);
    std::cout << "Note: this version emits Gill IR, not machine code yet.\n";
    return 0;
}
