#!/usr/bin/env sh

# Source this file before working on the LLVM backend:
#   . scripts/dev_env.sh

export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/llvm:${CMAKE_PREFIX_PATH:-}"
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib ${LDFLAGS:-}"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include ${CPPFLAGS:-}"

