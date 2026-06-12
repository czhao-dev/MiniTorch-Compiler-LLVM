#pragma once

#include "graph.h"

namespace minitorch::passes {

void eliminateDeadCode(Graph &graph);

} // namespace minitorch::passes

