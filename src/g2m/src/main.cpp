#include "ir.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace fs = std::filesystem;

static std::string quote(const std::string& s) {
#ifdef _WIN32
    std::string out = "\"";
    for (char c : s) { if (c == '"') out += '\\'; out += c; }
    return out + "\"";
#else
    std::string out = "'";
    for (char c : s) { if (c == '\'') out += "'\\''"; else out += c; }
    return out + "'";
#endif
}

static bool emitCpp(const g2m::Program& p, const fs::path& path, std::string& why) {
    std::ofstream out(path);
    if (!out) { why = "cannot write temporary C++ source"; return false; }
    const g2m::Function* mainFn = nullptr;
    for (const auto& f : p.functions) {
        if (f.declarationOnly) continue;
        if (f.name == "main") mainFn = &f;
        if (f.name != "main" || f.returnType != "nil" || !f.parameters.empty()) {
            why = "this build supports a zero-argument fn main() nil entry point; other function bodies are not yet emitted";
            return false;
        }
    }
    if (!mainFn) { why = "program must define fn main() nil"; return false; }
    out << "#include <cstdint>\n#include <iostream>\n#include <string>\n#include <cmath>\n\n";
    for (const auto& f : p.functions) {
        if (f.declarationOnly) continue;
        if (f.name != "main") {
            why = "only main() is currently supported as a function definition";
            return false;
        }
        out << "int main() {\n";
        for (const auto& s : f.body) {
            using K = g2m::Statement::Kind;
            if (s.kind == K::VariableI32) {
                static const std::unordered_map<std::string,std::string> types = {
                    {"i8","std::int8_t"},{"i16","std::int16_t"},{"i32","std::int32_t"},{"i64","std::int64_t"},{"i128","__int128"},
                    {"u8","std::uint8_t"},{"u16","std::uint16_t"},{"u32","std::uint32_t"},{"u64","std::uint64_t"},{"u128","unsigned __int128"},
                    {"isize","std::intptr_t"},{"usize","std::uintptr_t"},{"int","std::int32_t"},{"f32","float"},{"f64","double"},
                    {"bool","bool"},{"char","char"},{"string","std::string"}};
                auto it=types.find(s.declaredType); if(it==types.end()){why="unsupported variable type '"+s.declaredType+"'";return false;}
                out << "  " << (s.constant ? "const " : "") << it->second << " " << s.name << " = " << s.value << ";\n";
            } else if (s.kind == K::PrintString) {
                if (s.arguments.size() != 1) { why = "invalid printn call"; return false; }
                out << "  std::cout << " << s.arguments[0].value << " << '\\n';\n";
            } else if (s.kind == K::Printf) {
                out << "  ";
                for (const auto& a : s.arguments) out << "std::cout << " << a.value << "; ";
                if(s.name=="printfn") out << "std::cout << '\\n';";
                out << "\n";
            } else {
                why = "function calls other than printn/printfn are not yet supported by the native backend";
                return false;
            }
        }
        out << "  return 0;\n}\n";
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: g2m <file.gill> [-c output] [-Ro]\n";
        return 1;
    }
    std::string input = argv[1], output;
    bool release = false;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c") {
            if (++i >= argc) { std::cerr << "g2m: -c requires an output name\n"; return 1; }
            output = argv[i];
        } else if (arg == "-Ro") release = true;
        else { std::cerr << "g2m: unknown option '" << arg << "'\n"; return 1; }
    }
    if (output.empty()) output = fs::path(input).stem().string();
    std::ifstream file(input);
    if (!file) { std::cerr << "g2m: cannot open '" << input << "'\n"; return 1; }
    const std::string source((std::istreambuf_iterator<char>(file)), {});
    g2m::Lexer lexer(source);
    auto tokens = lexer.scan();
    for (const auto& e : lexer.errors()) std::cerr << "g2m: " << e << '\n';
    if (!lexer.errors().empty()) return 1;
    g2m::Parser parser(tokens);
    auto program = parser.parse();
    for (const auto& e : parser.errors()) std::cerr << "g2m: " << e << '\n';
    if (!parser.errors().empty()) return 1;

    bool hasIO = false;
    for (const auto& i : program.imports) if (i == "stdlib" || i == "stdlib::io") hasIO = true;
    for (const auto& f : program.functions) for (const auto& s : f.body)
        if ((s.kind == g2m::Statement::Kind::PrintString || s.kind == g2m::Statement::Kind::Printf) && !hasIO) {
            std::cerr << "g2m: output functions require 'import stdlib;' or 'import stdlib::io;'\n"; return 1;
        }
    std::unordered_set<std::string> names;
    for (const auto& f : program.functions) {
        if (!names.insert(f.name).second) { std::cerr << "g2m: duplicate function '" << f.name << "'\n"; return 1; }
        for (std::size_t i = 0; i < f.parameters.size(); ++i)
            if (f.parameters[i].variadic && i + 1 != f.parameters.size()) {
                std::cerr << "g2m: variadic parameter must be final in '" << f.name << "'\n"; return 1;
            }
    }

    fs::path temp = fs::path(output);
#ifdef _WIN32
    fs::path executable = temp; if (executable.extension() != ".exe") executable += ".exe";
    fs::path cpp = temp; cpp += ".g2m.cpp";
#else
    fs::path executable = temp;
    fs::path cpp = temp; cpp += ".g2m.cpp";
#endif
    std::string why;
    if (!emitCpp(program, cpp, why)) {
        std::cerr << "g2m: " << why << '\n';
        std::error_code ec; fs::remove(cpp, ec);
        return 1;
    }
    const char* configured = std::getenv("CXX");
#ifdef _WIN32
    std::string compiler = configured ? configured : "c++";
#else
    std::string compiler = configured ? configured : "c++";
#endif
    std::string command = quote(compiler) + " " + quote(cpp.string()) + (release ? " -O2 -DNDEBUG" : " -O0 -g") + " -o " + quote(executable.string());
    int status = std::system(command.c_str());
    std::error_code ec; fs::remove(cpp, ec);
    if (status != 0) {
        std::cerr << "g2m: native C++ compiler failed (install a C++20 compiler or set CXX)\n";
        return 1;
    }
    std::cout << "g2m: created executable '" << executable.string() << "'\n";
    return 0;
}
