cmake_minimum_required(VERSION 3.20)
project(g2m VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(g2m
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ir.cpp
)
target_include_directories(g2m PRIVATE include)

if(MSVC)
    target_compile_options(g2m PRIVATE /W4 /permissive-)
else()
    target_compile_options(g2m PRIVATE -Wall -Wextra -Wpedantic)
endif()
