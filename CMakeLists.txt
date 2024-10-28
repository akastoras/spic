cmake_minimum_required(VERSION 3.20)
project(spic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SRC_DIR "src")
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${BUILD_DIR}/lexer.cpp")
set(PARSER_OUT "${BUILD_DIR}/parser.cpp")

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${BUILD_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${BUILD_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_executable(spic "${SRC_DIR}/main.cpp" "${LEXER_OUT}" "${PARSER_OUT}")
target_include_directories(spic PRIVATE "${BUILD_DIR}")