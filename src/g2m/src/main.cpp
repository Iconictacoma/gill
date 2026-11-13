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
static std::string cppType(const std::string&t){static const std::unordered_map<std::string,std::string> m={{"i8","std::int8_t"},{"i16","std::int16_t"},{"i32","std::int32_t"},{"int","std::int32_t"},{"i64","std::int64_t"},{"i128","__int128"},{"u8","std::uint8_t"},{"u16","std::uint16_t"},{"u32","std::uint32_t"},{"u64","std::uint64_t"},{"u128","unsigned __int128"},{"isize","std::intptr_t"},{"usize","std::uintptr_t"},{"f32","float"},{"f64","double"},{"bool","bool"},{"char","char"},{"string","std::string"},{"string[]","std::vector<std::string>"},{"zString*","char**"},{"nil","void"}};auto i=m.find(t);return i==m.end()?"auto":i->second;}
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
 bool withArgs=false; if(!main->parameters.empty()){
 if(main->parameters.size()!=2 || main->parameters[0].type!="i32" ||
   (main->parameters[1].type!="string[]" && main->parameters[1].type!="zString*")){
   err="main parameters must be (), (i32 argc, string[] argv), or (i32 argc, zString* argv)";return false;
 } withArgs=true;
 }
 std::ofstream o(path);if(!o){err="cannot write temporary C++ source";return false;}
 o<<"#include <cstdint>\n#include <iostream>\n#include <string>\n#include <cmath>\n#include <cstdlib>\n#include <vector>\n#include <algorithm>\n#include <limits>\n#include <sstream>\n";
 o<<R"CPP(
#include <iomanip>
#include <type_traits>
#include <stdexcept>
#include <cctype>
#define printf gill_printf
template<class T> constexpr bool fmt_ok(char c) {
 using V=std::decay_t<T>;
 if(c=='d') return std::is_integral_v<V> && std::is_signed_v<V>;
 if(c=='u'||c=='x'||c=='o') return std::is_integral_v<V> && std::is_unsigned_v<V>;
 if(c=='f') return std::is_floating_point_v<V>;
 if(c=='c') return std::is_same_v<V,char>;
 if(c=='s') return std::is_same_v<V,std::string>;
 if(c=='b') return std::is_same_v<V,bool>;
 return false;
}
inline void emit_fmt(std::ostream& out,const std::string& f,std::size_t& i) {
 while(i<f.size()){if(f[i]=='%'&&i+1<f.size()&&f[i+1]=='%'){out<<'%';i+=2;}else if(f[i]=='%')throw std::runtime_error("too few format arguments");else out<<f[i++];}
}
template<class T,class... R> void emit_fmt(std::ostream& out,const std::string& f,std::size_t& i,const T& value,const R&... rest) {
 while(i<f.size()){
   if(f[i]=='%'&&i+1<f.size()&&f[i+1]=='%'){out<<'%';i+=2;continue;}
   if(f[i]=='%')break;
   if(f[i]=='\\'&&i+1<f.size()){char e=f[i+1];out<<(e=='n'?'\\n':e=='t'?'\\t':e);i+=2;continue;}
   out<<f[i++];
 }
 if(i>=f.size())throw std::runtime_error("too many format arguments");
 ++i;
 int width=0,precision=-1;bool left=false,zero=false;
 if(i<f.size()&&f[i]=='-'){left=true;++i;}if(i<f.size()&&f[i]=='0'){zero=true;++i;}
 while(i<f.size()&&std::isdigit((unsigned char)f[i]))width=width*10+(f[i++]-'0');
 if(i<f.size()&&f[i]=='.'){++i;precision=0;while(i<f.size()&&std::isdigit((unsigned char)f[i]))precision=precision*10+(f[i++]-'0');}
 if(i>=f.size())throw std::runtime_error("incomplete format identifier");
 char c=f[i++];if(!fmt_ok<T>(c))throw std::runtime_error(std::string("format type mismatch for %")+c);
 std::ostringstream tmp;if(precision>=0)tmp<<std::fixed<<std::setprecision(precision);
 if(c=='x')tmp<<std::hex;if(c=='o')tmp<<std::oct;
 if constexpr(std::is_same_v<std::decay_t<T>,bool>)tmp<<(value?"true":"false");else tmp<<value;
 std::string v=tmp.str();if(width>(int)v.size()){std::string pad(width-v.size(),zero?'0':' ');v=left?v+pad:pad+v;}out<<v;
 emit_fmt(out,f,i,rest...);
}
template<class... T> void printf(const std::string& f,const T&... x){std::size_t i=0;emit_fmt(std::cout,f,i,x...);}
template<class... T> void printfn(const std::string& f,const T&... x){printf(f,x...);std::cout<<'\n';}
template<class T> void printn(const T& x){std::cout<<x<<'\n';}
)CPP";
 o<<"namespace gill { template<class T> auto length(const T& x){return x.size();} inline int parse_i32(const std::string&s){return std::stoi(s);} inline double parse_f64(const std::string&s){return std::stod(s);} template<class T> auto absolute(T x){return std::abs(x);} inline double sqrt(double x){return std::sqrt(x);} inline double pow(double a,double b){return std::pow(a,b);} }\n";
 for(auto&f:p.functions)if(!f.declarationOnly&&f.name!="main"){o<<cppType(f.returnType)<<" "<<f.name<<"(";for(std::size_t i=0;i<f.parameters.size();++i){if(i)o<<",";o<<cppType(f.parameters[i].type)<<" "<<f.parameters[i].name;}o<<");\n";}
 for(auto&f:p.functions)if(!f.declarationOnly&&f.name!="main"){o<<cppType(f.returnType)<<" "<<f.name<<"(";for(std::size_t i=0;i<f.parameters.size();++i){if(i)o<<",";o<<cppType(f.parameters[i].type)<<" "<<f.parameters[i].name;}o<<"){\n";for(auto&s:f.body)emitStmt(o,s,2);if(f.returnType=="nil")o<<"  return;\n";o<<"}\n";}
 if(withArgs && main->parameters[1].type=="string[]") o<<"int main(int g2m_argc,char** g2m_argv){ std::vector<std::string> gill_argv; for(int i=0;i<g2m_argc;++i) gill_argv.emplace_back(g2m_argv[i]);\n";
 else if(withArgs) o<<"int main(int g2m_argc,char** g2m_argv){\n";
 else o<<"int main(){\n";
 if(withArgs){o<<"  "<<cppType(main->parameters[0].type)<<" "<<main->parameters[0].name<<" = g2m_argc;\n";if(main->parameters[1].type=="string[]")o<<"  auto "<<main->parameters[1].name<<" = gill_argv;\n";else o<<"  char** "<<main->parameters[1].name<<" = g2m_argv;\n";}
 for(auto&s:main->body)emitStmt(o,s,2);if(main->returnType=="nil")o<<"  return 0;\n";else o<<"  return 0;\n";o<<"}\n";return true;
}
int main(int argc,char**argv){if(argc<2){std::cerr<<"Usage: g2m <file.gill> [-c output] [-Ro]\n";return 1;}std::string input=argv[1],output;bool release=false;for(int i=2;i<argc;++i){std::string a=argv[i];if(a=="-c"){if(++i>=argc){std::cerr<<"g2m: -c requires output name\n";return 1;}output=argv[i];}else if(a=="-Ro")release=true;else{std::cerr<<"g2m: unknown option "<<a<<"\n";return 1;}}if(output.empty())output=fs::path(input).stem().string();
 std::ifstream in(input);if(!in){std::cerr<<"g2m: cannot open "<<input<<"\n";return 1;}std::string src((std::istreambuf_iterator<char>(in)),{});g2m::Lexer lex(src);auto ts=lex.scan();for(auto&e:lex.errors())std::cerr<<"g2m: "<<e<<"\n";if(!lex.errors().empty())return 1;g2m::Parser parser(ts);auto p=parser.parse();for(auto&e:parser.errors())std::cerr<<"g2m: "<<e<<"\n";if(!parser.errors().empty())return 1;
 fs::path cpp=output+".g2m.cpp";std::string err;if(!emit(p,cpp,err)){std::cerr<<"g2m: "<<err<<"\n";return 1;}const char*cxx=std::getenv("CXX");std::string cmd=shquote(cxx?cxx:"c++")+" "+shquote(cpp.string())+(release?" -std=c++20 -O2 -DNDEBUG":" -std=c++20 -O0 -g")+" -o "+shquote(output);int rc=std::system(cmd.c_str());std::error_code ec;fs::remove(cpp,ec);if(rc){std::cerr<<"g2m: host C++ compiler failed\n";return 1;}std::cout<<"g2m: created executable '"<<output<<"'\n";return 0;}
