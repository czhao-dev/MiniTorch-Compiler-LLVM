# Build Notes

## macOS

```bash
brew install cmake llvm
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

If `llvm-config` is not on `PATH`, use Homebrew's LLVM directly:

```bash
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
cmake -S . -B build-llvm -DMINITORCH_ENABLE_LLVM=ON -DLLVM_DIR="$(llvm-config --cmakedir)"
```

The current front-end milestone builds with `MINITORCH_ENABLE_LLVM=OFF`, which
is the default. Turn LLVM on when implementing `src/codegen.cpp`.

## Linux

```bash
sudo apt install cmake llvm clang
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

