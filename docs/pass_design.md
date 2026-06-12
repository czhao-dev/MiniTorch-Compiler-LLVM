# Optimization Pass Design

MiniTorch's first optimization passes run on the computation graph instead of
raw LLVM IR. At graph level, tensor operation semantics and shapes are explicit,
so transformations such as fusion and CSE are easier to implement and test.

## Constant Folding

Constant folding replaces operations whose inputs are all compile-time constants
with a single constant node. This removes redundant scalar or shape-related
work before code generation.

## Dead Code Elimination

DCE starts at the graph output nodes and walks backward through input edges.
Nodes not reached by that traversal cannot affect the returned tensor and can
be removed.

## Operator Fusion

Fusion detects patterns such as `MATMUL -> RELU` or `MATMUL -> SIGMOID` where
the intermediate matmul output has a single consumer. The pair is replaced with
a fused op that codegen can emit as one loop nest.

## Common Subexpression Elimination

CSE hashes a node by its operation and ordered input IDs. If an equivalent node
already exists, later uses can be redirected to the original node.

## Deferred Work

Loop tiling and vectorization are intentionally deferred. Those optimizations
are valuable, but they need a working end-to-end codegen path first.

