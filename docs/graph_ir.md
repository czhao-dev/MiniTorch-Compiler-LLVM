# Computation Graph IR

The graph IR is the compiler layer between typed AST nodes and LLVM IR. It
represents tensor operations as explicit data-flow nodes, which makes graph
optimizations easier to reason about than source-level rewrites.

## Node Shape

Each graph node contains:

- `id`: stable integer node ID
- `op`: operation enum such as `INPUT`, `MATMUL`, `ADD`, or `OUTPUT`
- `name`: optional source-level name for debugging
- `inputs`: ordered list of input node IDs
- `type`: statically inferred tensor shape

## Planned Op Set

- `INPUT`
- `OUTPUT`
- `CONST`
- `MATMUL`
- `ADD`
- `RELU`
- `SIGMOID`
- `SOFTMAX`
- `TRANSPOSE`
- `RESHAPE`
- `MATMUL_RELU`
- `MATMUL_SIGMOID`

## Lowering Plan

Function parameters become `INPUT` nodes. Expression lowering recursively emits
operation nodes and stores assignment aliases in a symbol table. A return
statement emits an `OUTPUT` node pointing at the returned expression. Once every
node has a shape, optimization passes can transform the graph before LLVM IR is
generated.

