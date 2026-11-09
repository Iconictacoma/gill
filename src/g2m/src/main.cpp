#include "lexer.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
namespace fs=std::filesystem;
static std::string shquote(const std::string&s){std::string o="'";for(char c:s){if(c=='\'')o+="'\\''";else o+=c;}return o+"'";}
static std::string cppType(const std::string&t){static const std::unordered_map<std::string,std::string> m={{"i8","std::int8_t"},{"i16","std::int16_t"},{"i32","std::int32_t"},{"int","std::int32_t"},{"i64","std::int64_t"},{"i128","__int128"},{"u8","std::uint8_t"},{"u16","std::uint16_t"},{"u32","std::uint32_t"},{"u64","std::uint64_t"},{"u128","unsigned __int128"},{"isize","std::intptr_t"},{"usize","std::uintptr_t"},{"f32","float"},{"f64","double"},{"bool","bool"},{"char","char"},{"string","std::string"},{"nil","void"}};auto i=m.find(t);return i==m.end()?"auto":i->second;}
static void emitStmt(std::ostream&o,const g2m::Statement&s,int d){
 std::string pad(d,' ');using K=g2m::Statement::Kind;
 switch(s.kind){
 case K::Variable:o<<pad<<(s.constant?"const ":"")<<cppType(s.declaredType)<<" "<<s.name<<" = "<<s.value<<";\n";break;
 case K::Expression:o<<pad<<s.value<<";\n";break;
 case K::Return:o<<pad<<"return"<<(s.value.empty()?"":" "+s.value)<<";\n";break;
 case K::Break:o<<pad<<"break;\n";break;
 case K::Continue:o<<pad<<"continue;\n";break;
 case K::Block:o<<pad<<"{\n";for(auto&x:s.body)emitStmt(o,x,d+2);o<<pad<<"}\n";break;
 case K::If:o<<pad<<"if ("<<s.condition<<") {\n";for(auto&x:s.body)emitStmt(o,x,d+2);o<<pad<<"}";if(!s.otherwise.empty()){o<<" else {\n";for(auto&x:s.otherwise)emitStmt(o,x,d+2);o<<pad<<"}";}o<<"\n";break;
 case K::While:o<<pad<<"while ("<<s.condition<<") {\n";for(auto&x:s.body)emitStmt(o,x,d+2);o<<pad<<"}\n";break;
 case K::For:{std::string init=s.name;static const std::unordered_map<std::string,std::string> aliases={{"i8","std::int8_t"},{"i16","std::int16_t"},{"i32","std::int32_t"},{"int","std::int32_t"},{"i64","std::int64_t"},{"u8","std::uint8_t"},{"u16","std::uint16_t"},{"u32","std::uint32_t"},{"u64","std::uint64_t"},{"f32","float"},{"f64","double"},{"bool","bool"},{"char","char"},{"string","std::string"}};for(auto&[a,b]:aliases){if(init.rfind(a+" ",0)==0){init.replace(0,a.size(),b);break;}}o<<pad<<"for ("<<init<<"; "<<s.condition<<"; "<<s.increment<<") {\n";for(auto&x:s.body)emitStmt(o,x,d+2);o<<pad<<"}\n";break;}
 }
}
static bool emit(const g2m::Program&p,const fs::path&path,std::string&err){
 const g2m::Function*main=nullptr;std::unordered_set<std::string>names;
 for(auto&f:p.functions){if(!names.insert(f.name).second){err="duplicate function '"+f.name+"'";return false;}if(f.name=="main"&&!f.declarationOnly)main=&f;}
 if(!main){err="missing fn main entry point";return false;}
 if(!(main->returnType=="nil"||main->returnType=="i32"||main->returnType=="int")){err="main must return nil, i32, or int";return false;}
 if(!main->parameters.empty()){err="this build supports main() without parameters; argc/argv support is next";return false;}
 std::ofstream o(path);if(!o){err="cannot write temporary C++ source";return false;}
 o<<"#include <cstdint>\n#include <iostream>\n#include <string>\n#include <cmath>\n#include <cstdlib>\n#include <vector>\n#include <algorithm>\n#include <limits>\n#include <sstream>\n";
 o<<"template<class... T> void printfn(const T&... x){(std::cout << ... << x);std::cout<<'\\n';}\n";
 o<<"template<class T> void printn(const T& x){std::cout<<x<<'\\n';}\n";
 o<<"namespace gill { template<class T> auto length(const T& x){return x.size();} inline int parse_i32(const std::string&s){return std::stoi(s);} inline double parse_f64(const std::string&s){return std::stod(s);} template<class T> auto absolute(T x){return std::abs(x);} inline double sqrt(double x){return std::sqrt(x);} inline double pow(double a,double b){return std::pow(a,b);} }\n";
 for(auto&f:p.functions)if(!f.declarationOnly&&f.name!="main"){o<<cppType(f.returnType)<<" "<<f.name<<"(";for(std::size_t i=0;i<f.parameters.size();++i){if(i)o<<",";o<<cppType(f.parameters[i].type)<<" "<<f.parameters[i].name;}o<<");\n";}
 for(auto&f:p.functions)if(!f.declarationOnly&&f.name!="main"){o<<cppType(f.returnType)<<" "<<f.name<<"(";for(std::size_t i=0;i<f.parameters.size();++i){if(i)o<<",";o<<cppType(f.parameters[i].type)<<" "<<f.parameters[i].name;}o<<"){\n";for(auto&s:f.body)emitStmt(o,s,2);if(f.returnType=="nil")o<<"  return;\n";o<<"}\n";}
 o<<"int main(){\n";for(auto&s:main->body)emitStmt(o,s,2);if(main->returnType=="nil")o<<"  return 0;\n";else o<<"  return 0;\n";o<<"}\n";return true;
}
int main(int argc,char**argv){if(argc<2){std::cerr<<"Usage: g2m <file.gill> [-c output] [-Ro]\\n";return 1;}std::string input=argv[1],output;bool release=false;for(int i=2;i<argc;++i){std::string a=argv[i];if(a=="-c"){if(++i>=argc){std::cerr<<"g2m: -c requires output name\\n";return 1;}output=argv[i];}else if(a=="-Ro")release=true;else{std::cerr<<"g2m: unknown option "<<a<<"\\n";return 1;}}if(output.empty())output=fs::path(input).stem().string();
 std::ifstream in(input);if(!in){std::cerr<<"g2m: cannot open "<<input<<"\\n";return 1;}std::string src((std::istreambuf_iterator<char>(in)),{});g2m::Lexer lex(src);auto ts=lex.scan();for(auto&e:lex.errors())std::cerr<<"g2m: "<<e<<"\n";if(!lex.errors().empty())return 1;g2m::Parser parser(ts);auto p=parser.parse();for(auto&e:parser.errors())std::cerr<<"g2m: "<<e<<"\n";if(!parser.errors().empty())return 1;
 fs::path cpp=output+".g2m.cpp";std::string err;if(!emit(p,cpp,err)){std::cerr<<"g2m: "<<err<<"\n";return 1;}const char*cxx=std::getenv("CXX");std::string cmd=shquote(cxx?cxx:"c++")+" "+shquote(cpp.string())+(release?" -std=c++20 -O2 -DNDEBUG":" -std=c++20 -O0 -g")+" -o "+shquote(output);int rc=std::system(cmd.c_str());std::error_code ec;fs::remove(cpp,ec);if(rc){std::cerr<<"g2m: host C++ compiler failed\n";return 1;}std::cout<<"g2m: created executable '"<<output<<"'\n";return 0;}
