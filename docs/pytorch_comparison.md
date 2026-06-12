# PyTorch Comparison

MiniTorch is inspired by production systems such as TorchScript,
`torch.compile`, ONNX Runtime, and XLA, but it is intentionally much smaller.

## Similarities

- Parses a restricted, analyzable representation of model code
- Uses static tensor information to catch errors before execution
- Lowers high-level tensor operations toward native execution
- Avoids Python interpreter overhead in the intended hot path

## Differences

- MiniTorch supports a fixed operation set instead of the full PyTorch API
- Shapes are static by design
- The project emphasizes compiler pipeline clarity over broad model coverage
- It does not include PyTorch's serialization, autograd, or dynamic control flow

## Why Build This

The value of MiniTorch is educational and portfolio-oriented: it demonstrates
the path from Python-like model code to IR, graph optimization, LLVM lowering,
and native execution in a compact codebase.

