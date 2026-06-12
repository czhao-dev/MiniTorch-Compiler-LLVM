#include "codegen.h"

#include <ostream>

namespace minitorch {

void CodeGenerator::emitPlaceholderIR(const Graph &graph, std::ostream &out) const {
    out << "; MiniTorch placeholder LLVM IR\n";
    out << "; Graph nodes: " << graph.nodes().size() << "\n";
    out << "; Enable MINITORCH_ENABLE_LLVM and implement src/codegen.cpp for native output.\n";
}

} // namespace minitorch

