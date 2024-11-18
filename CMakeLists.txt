cmake_minimum_required(VERSION 3.20)
project(spic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(SRC_DIR "src")
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_BUILD_TYPE Debug)

set(LEXER_OUT "${BUILD_DIR}/lexer.cpp")
set(PARSER_OUT "${BUILD_DIR}/parser.cpp")

set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen")
add_subdirectory(${EIGEN3_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/eigen_build)

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

include_directories(include)
include_directories(${EIGEN3_INCLUDE_DIR})

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${BUILD_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${BUILD_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

# Collect all .cpp files in the src directory
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp")

add_executable(spic ${SRC_FILES} "${LEXER_OUT}" "${PARSER_OUT}")

target_include_directories(spic PRIVATE "${BUILD_DIR}")
target_link_libraries(spic Eigen3::Eigen)


# Check if MY_MACRO is defined and add it to the target
if(DEFINED VERBOSITY)
	if (VERBOSITY EQUAL 1)
		target_compile_definitions(spic PRIVATE VERBOSE_NETLIST=2)
		target_compile_definitions(spic PRIVATE VERBOSE_NODETABLE=0)
		target_compile_definitions(spic PRIVATE VERBOSE_LEXER=0)
	elseif(VERBOSITY EQUAL 2)
		target_compile_definitions(spic PRIVATE VERBOSE_NETLIST=2)
		target_compile_definitions(spic PRIVATE VERBOSE_NODETABLE=1)
		target_compile_definitions(spic PRIVATE VERBOSE_LEXER=0)
	elseif(VERBOSITY EQUAL 3)
		target_compile_definitions(spic PRIVATE VERBOSE_NETLIST=2)
		target_compile_definitions(spic PRIVATE VERBOSE_NODETABLE=1)
		target_compile_definitions(spic PRIVATE VERBOSE_LEXER=1)
	else()
		target_compile_definitions(spic PRIVATE VERBOSE_NETLIST=1)
		target_compile_definitions(spic PRIVATE VERBOSE_NODETABLE=0)
		target_compile_definitions(spic PRIVATE VERBOSE_LEXER=0)
	endif()

	unset(VERBOSITY CACHE)
else()
	target_compile_definitions(spic PRIVATE VERBOSE_NETLIST=1)
	target_compile_definitions(spic PRIVATE VERBOSE_NODETABLE=0)
	target_compile_definitions(spic PRIVATE VERBOSE_LEXER=0)
endif()