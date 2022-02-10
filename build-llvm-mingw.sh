#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source ${MY_DIR}/common.sh

MY_OUTPUT_DIR="${ARTIFACTS_DIR}/llvm-mingw/${HOST}"
LLVM_MINGW_DIR="${MY_DIR}/external/llvm-mingw"

create_dir "${MY_OUTPUT_DIR}"

(cd "${LLVM_MINGW_DIR}"; ./build-all.sh "${MY_OUTPUT_DIR}")
