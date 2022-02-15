#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source ${MY_DIR}/common.sh

MY_OUTPUT_DIR="${ARTIFACTS_DIR}/llvm-mingw/${HOST}"
LLVM_MINGW_DIR="${MY_DIR}/external/llvm-mingw"

function prepare_sources()
{
	git reset --hard HEAD
	for s in *.sh; do
		sed -E -e "s/(macosx-version-min|CMAKE_OSX_DEPLOYMENT_TARGET)=[0-9]+\.[0-9]+/\1=${MACOS_TARGET}/g" < "${s}" > "${s}.new"
		mv "${s}.new" "${s}"
		chmod 755 "${s}"
	done
}

create_dir "${MY_OUTPUT_DIR}"

(cd "${LLVM_MINGW_DIR}"; prepare_sources)
(cd "${LLVM_MINGW_DIR}"; ./build-all.sh "${MY_OUTPUT_DIR}")
