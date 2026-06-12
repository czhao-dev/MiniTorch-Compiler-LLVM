#include "ast.h"
#include "codegen.h"
#include "graph.h"
#include "lexer.h"
#include "parser.h"
#include "typechecker.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void printUsage(std::ostream &out) {
    out << "usage: mtc <source.py> [--emit-tokens] [--emit-ast] [--emit-graph] [--emit-ir]\n"
        << "\n"
        << "MiniTorch compiler front-end scaffold.\n";
}

std::string readFile(const std::string &path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to open '" + path + "'");
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // namespace

int main(int argc, char **argv) {
    if (argc < 2) {
        printUsage(std::cerr);
        return 2;
    }

    std::string sourcePath;
    bool emitTokens = false;
    bool emitAst = false;
    bool emitGraph = false;
    bool emitIr = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(std::cout);
            return 0;
        }
        if (arg == "--emit-tokens") {
            emitTokens = true;
        } else if (arg == "--emit-ast") {
            emitAst = true;
        } else if (arg == "--emit-graph") {
            emitGraph = true;
        } else if (arg == "--emit-ir") {
            emitIr = true;
        } else if (sourcePath.empty()) {
            sourcePath = arg;
        } else {
            std::cerr << "unexpected argument: " << arg << "\n";
            return 2;
        }
    }

    if (sourcePath.empty()) {
        printUsage(std::cerr);
        return 2;
    }

    try {
        const std::string source = readFile(sourcePath);
        minitorch::Lexer lexer(source);
        std::vector<minitorch::Token> tokens = lexer.tokenize();

        if (emitTokens) {
            for (const auto &token : tokens) {
                std::cout << token.line << ":" << token.column << " "
                          << minitorch::tokenTypeName(token.type);
                if (!token.lexeme.empty()) {
                    std::cout << " '" << token.lexeme << "'";
                }
                std::cout << "\n";
            }
        }

        minitorch::Parser parser(std::move(tokens));
        minitorch::ModuleNode module = parser.parseModule();
        minitorch::TypeChecker checker;
        checker.check(module);

        if (emitAst) {
            minitorch::printAst(module, std::cout);
        }

        minitorch::GraphBuilder graphBuilder;
        minitorch::Graph graph = graphBuilder.build(module);

        if (emitGraph) {
            graph.print(std::cout);
        }

        if (emitIr) {
            minitorch::CodeGenerator codeGenerator;
            codeGenerator.emitPlaceholderIR(graph, std::cout);
        }

        if (!emitTokens && !emitAst && !emitGraph && !emitIr) {
            std::cout << "Parsed " << module.functions.size()
                      << " function(s). Native code generation is the next milestone.\n";
        }

        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "mtc: " << ex.what() << "\n";
        return 1;
    }
}

