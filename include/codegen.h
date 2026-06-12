#pragma once

#include "graph.h"

#include <ostream>

namespace minitorch {

class CodeGenerator {
public:
    void emitPlaceholderIR(const Graph &graph, std::ostream &out) const;
};

} // namespace minitorch

