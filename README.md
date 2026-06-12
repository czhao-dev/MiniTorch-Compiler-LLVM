# MiniTorch Compiler

> An ahead-of-time (AOT) compiler for a statically-typed subset of Python with
> PyTorch-style tensor operations — parsing real Python syntax, inferring tensor
> shapes, and compiling to optimized native binaries via LLVM IR.
> No PyTorch runtime required at inference time.

---

## Overview

MiniTorch takes Python functions annotated with tensor types and compiles them
directly to native machine code through a full compiler pipeline: a Python
subset lexer and parser, a tensor-aware type checker with shape inference,
a computation graph IR, four custom LLVM optimization passes, and an LLVM
backend targeting x86 and ARM.

---

## Current Status

- Lexer handles decorators, Python indentation, type annotations, calls,
  arithmetic operators, and basic punctuation.
- Parser handles decorated function definitions, tensor annotations,
  assignments, returns, calls, identifiers, numbers, and `+` / `*` expressions.
- Graph, type checker, codegen, and optimization passes are present as
  extension points for the next phases.

---

## Quick Start

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure

./build/mtc examples/mlp_forward.py --emit-tokens
./build/mtc examples/mlp_forward.py --emit-ast
```

LLVM support is intentionally optional while the back end is being built:

```bash
. scripts/dev_env.sh
cmake -S . -B build-llvm -DMINITORCH_ENABLE_LLVM=ON -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build build-llvm --parallel
```

---

## What MiniTorch Compiles

MiniTorch accepts Python functions that use a fixed set of PyTorch-style
operations and static tensor type annotations:

```python
# examples/mlp_forward.py

@minitorch.compile
def linear(x: Tensor[128, 256], w: Tensor[256, 512],
           b: Tensor[512]) -> Tensor[128, 512]:
    return matmul(x, w) + b

@minitorch.compile
def mlp_forward(x: Tensor[128, 256]) -> Tensor[128, 10]:
    h1 = relu(linear(x, W1, b1))
    h2 = relu(linear(h1, W2, b2))
    return linear(h2, W3, b3)
```

Supported operations: `matmul`, `add` (`+`), `relu`, `sigmoid`, `softmax`,
`transpose`, `reshape`. Supported types: `Tensor[d0, d1, ...]` with static
integer dimensions. Supported control flow: assignments, `return`, `if`/`else`
(no loops in the first version — loops are a stretch goal).

Shapes are fully known at compile time, enabling dimension mismatch errors
before a single byte of native code is generated — the same guarantee
TorchScript provides.

---

## Pipeline Architecture

```
Python source (.py)
        │
        ▼
    Lexer                tokenize Python syntax: def, return, if,
                         identifiers, operators, type annotations
        │
        ▼
    Parser               recursive descent → Python-subset AST
                         handles decorators, type hints, assignments
        │
        ▼
    Type Checker         shape inference across all operations
    + Shape Inference    catches dimension mismatches at compile time
        │
        ▼
    Graph Lowering       typed AST → computation graph IR
                         nodes are tensor ops, edges are data flow
        │
        ▼
    Optimization Passes
    ├── Constant Folding         fold compile-time scalar expressions
    ├── Dead Code Elimination    remove unused tensor ops from the graph
    ├── Operator Fusion          fuse matmul + relu into one loop nest
    └── Common Subexpression     deduplicate identical sub-computations
        Elimination (CSE)
        │
        ▼
    LLVM IR Generator    lower computation graph to LLVM IR
                         tensors as flat float arrays, ops as loop nests
        │
        ▼
    LLVM Backend         x86 / ARM codegen via llc or LLJIT
        │
        ▼
    Native Binary + Benchmark
    (PyTorch eager  vs  MiniTorch no-opt  vs  MiniTorch optimized)
```

---

## Optimization Passes

### Constant Folding
Evaluates scalar expressions whose operands are compile-time constants.
Scaling a weight matrix by a fixed learning rate factor is folded into the
stored weights at compile time, eliminating the runtime multiply.

### Dead Code Elimination
Removes tensor operations in the computation graph whose outputs are never
consumed by a downstream node or the function's return value. Common after
operator fusion eliminates the consumers of intermediate results.

### Operator Fusion
The most impactful pass for ML workloads. Detects `matmul → relu` and
`matmul → sigmoid` patterns in the computation graph and replaces the two
nodes with a single fused node that emits one loop nest — computing the
activation in the same pass as the matrix multiply without writing the
intermediate result to memory. On a two-layer MLP this eliminates two
full tensor writes and reads per forward pass, which is the dominant
bottleneck on memory-bandwidth-limited hardware.

### Common Subexpression Elimination (CSE)
Detects when the same tensor operation with the same inputs appears more
than once in the computation graph and replaces the duplicate nodes with a
single shared node. Particularly useful in attention mechanisms where the
same key projection appears in multiple heads.

---

## Why AOT Over PyTorch Eager Mode

PyTorch's default eager mode executes each operation as Python calls it,
paying Python interpreter overhead per op and re-examining the computation
graph on every forward pass. MiniTorch compiles the graph once and produces
a native function pointer that can be called directly with no Python overhead,
no operator dispatch, and no dynamic shape checks — because all shapes are
statically known.

This tradeoff (static shapes for maximum performance) is exactly the design
point chosen by TorchScript, ONNX Runtime, and most on-device inference
runtimes for embedded and mobile targets.

---

## Benchmark Suite

| Experiment | PyTorch Eager (ms) | MiniTorch No-opt (ms) | MiniTorch Opt (ms) | Speedup vs Eager |
|---|---|---|---|---|
| matmul 256×256, batch 1 | — | — | — | — |
| matmul + relu (fused) | — | — | — | — |
| 2-layer MLP, batch 128 | — | — | — | — |
| 3-layer MLP, batch 512 | — | — | — | — |
| Fusion pass only | — | — | — | — |
| CSE pass only | — | — | — | — |

> Results logged to `benchmarks/results.csv`.
> Tested on: [machine + OS + LLVM version + PyTorch version].

---

## Repo Structure

```
minitorch-compiler/
├── README.md
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   ├── typechecker.h
│   ├── graph.h               ← computation graph IR
│   ├── codegen.h
│   └── passes/
│       ├── constant_fold.h
│       ├── dce.h
│       ├── op_fusion.h
│       └── cse.h
├── src/
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── typechecker.cpp
│   ├── graph.cpp
│   ├── codegen.cpp
│   ├── main.cpp
│   └── passes/
│       ├── constant_fold.cpp
│       ├── dce.cpp
│       ├── op_fusion.cpp
│       └── cse.cpp
├── python/
│   └── minitorch/
│       ├── __init__.py       ← exposes @minitorch.compile decorator
│       └── driver.py         ← extracts source, invokes compiler, loads .so
├── tests/
│   ├── lexer_test.cpp
│   ├── parser_test.cpp
│   ├── typechecker_test.cpp
│   ├── graph_test.cpp
│   └── codegen_test.cpp
├── examples/
│   ├── matmul.py
│   ├── mlp_forward.py
│   └── attention_head.py
├── benchmarks/
│   ├── bench.py              ← compares PyTorch eager vs MiniTorch
│   └── results.csv
└── docs/
    ├── language_spec.md      ← supported Python subset + BNF grammar
    ├── graph_ir.md            ← computation graph IR design
    ├── pass_design.md         ← how each pass works and why
    └── pytorch_comparison.md  ← how MiniTorch relates to TorchScript / XLA
```

---

## Build & Run

```bash
# Dependencies: LLVM 17+, CMake 3.20+, Python 3.10+, PyTorch (for benchmarks only)
brew install llvm cmake          # macOS
sudo apt install llvm cmake      # Ubuntu

# Build the compiler
mkdir build && cd build
cmake .. -DLLVM_DIR=$(llvm-config --cmakedir)
make -j$(nproc)

# Compile a Python file to a native shared library
./mtc examples/mlp_forward.py -o mlp_forward.so

# Dump computation graph IR
./mtc examples/mlp_forward.py --emit-graph

# Dump LLVM IR before passes
./mtc examples/mlp_forward.py --emit-ir

# Dump optimized LLVM IR
./mtc examples/mlp_forward.py --emit-ir --optimize

# Use from Python via the decorator
python3 -c "
import minitorch
import numpy as np
from examples.mlp_forward import mlp_forward
result = mlp_forward(np.random.randn(128, 256).astype('float32'))
print(result.shape)
"

# Run benchmark suite
python3 benchmarks/bench.py --output benchmarks/results.csv
```

---

## Tech Stack

- **Compiler:** C++17, LLVM 17 (IRBuilder, PassManager, LLJIT)
- **Python integration:** CPython C API or ctypes to call compiled `.so`
- **Build:** CMake
- **Testing:** Google Test (C++), pytest (Python integration tests)
- **Benchmarking:** `time.perf_counter` vs PyTorch `torch.utils.benchmark`

---

## How This Compares to TorchScript

TorchScript is PyTorch's production Python-subset compiler. MiniTorch is a
from-scratch reimplementation of the same idea at a smaller scale, built to
expose the compiler pipeline clearly rather than maximize op coverage.

Key similarities: both parse a static subset of Python, both perform type
checking and shape inference, both produce native code without the Python
interpreter in the hot path.

Key differences: TorchScript supports the full PyTorch op set, dynamic control
flow via loop unrolling, and serialization for mobile deployment. MiniTorch
supports a fixed op set, static shapes only, and focuses on exposing the
compiler pipeline clearly rather than maximizing coverage.

---

## Further Reading

- [TorchScript documentation](https://pytorch.org/docs/stable/jit.html)
- [torch.compile internals](https://pytorch.org/docs/stable/torch.compiler.html)
- [LLVM Language Reference Manual](https://llvm.org/docs/LangRef.html)
- [Writing an LLVM Pass](https://llvm.org/docs/WritingAnLLVMPass.html)
- [MLIR Rationale](https://mlir.llvm.org/docs/Rationale/Rationale/) —
  the next step after this project; MLIR is what production ML compilers
  use for the graph IR layer
- [Engineering a Compiler — Cooper & Torczon](https://www.elsevier.com/books/engineering-a-compiler/cooper/978-0-12-818128-4) —
  Chapters 5–9 cover the core techniques used here
