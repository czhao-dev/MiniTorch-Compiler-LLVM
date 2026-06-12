cmake_minimum_required(VERSION 3.20)

project(MiniTorchCompiler VERSION 0.1.0 LANGUAGES C CXX)

option(MINITORCH_ENABLE_LLVM "Enable LLVM-dependent code generation components" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minitorch_core
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/typechecker.cpp
    src/graph.cpp
    src/codegen.cpp
    src/passes/constant_fold.cpp
    src/passes/dce.cpp
    src/passes/op_fusion.cpp
    src/passes/cse.cpp
)

target_include_directories(minitorch_core PUBLIC include)
target_compile_options(minitorch_core PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall -Wextra -Wpedantic>
)

if(MINITORCH_ENABLE_LLVM)
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    target_include_directories(minitorch_core SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(minitorch_core PRIVATE ${LLVM_DEFINITIONS} MINITORCH_ENABLE_LLVM=1)
    llvm_map_components_to_libnames(LLVM_LIBS core support irreader native nativecodegen orcjit)
    target_link_libraries(minitorch_core PRIVATE ${LLVM_LIBS})
endif()

add_executable(mtc src/main.cpp)
target_link_libraries(mtc PRIVATE minitorch_core)

enable_testing()
add_executable(minitorch_tests tests/test_main.cpp)
target_link_libraries(minitorch_tests PRIVATE minitorch_core)
add_test(NAME minitorch_frontend_tests COMMAND minitorch_tests)
