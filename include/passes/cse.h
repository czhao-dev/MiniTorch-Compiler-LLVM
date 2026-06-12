#pragma once

#include "graph.h"

namespace minitorch::passes {

void eliminateCommonSubexpressions(Graph &graph);

} // namespace minitorch::passes

