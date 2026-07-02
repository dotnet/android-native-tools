#!/bin/bash

set -e

MY_DIR="$(cd "$(dirname "$0")"; pwd)"
LLVM_DIR="${MY_DIR}/external/llvm"

# The packaged toolchain builds LLVM plus lld only. Remove unused editor,
# documentation, test, and helper-tooling package metadata that is not needed by
# the build or package steps.
rm -rf \
  "${LLVM_DIR}/.ci" \
  "${LLVM_DIR}/clang/test/Analysis/LifetimeSafety" \
  "${LLVM_DIR}/clang/utils/analyzer" \
  "${LLVM_DIR}/flang/examples/FlangOmpReport" \
  "${LLVM_DIR}/lldb/test" \
  "${LLVM_DIR}/lldb/tools/lldb-dap" \
  "${LLVM_DIR}/llvm/docs" \
  "${LLVM_DIR}/llvm/utils/git" \
  "${LLVM_DIR}/llvm/utils/vscode" \
  "${LLVM_DIR}/mlir/python" \
  "${LLVM_DIR}/mlir/utils/mbr" \
  "${LLVM_DIR}/mlir/utils/tree-sitter-mlir" \
  "${LLVM_DIR}/mlir/utils/vscode" \
  "${LLVM_DIR}/third-party/benchmark/tools"
