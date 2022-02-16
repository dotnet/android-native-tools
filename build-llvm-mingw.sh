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

#
# This is equivalent to the build-all.sh script from llvm-mingw, but it allows us
# to override CMake flags per script
#
function build()
{
	local PREFIX="${1}"

	CMAKEFLAGS="-DLLDB_ENABLE_PYTHON=OFF" ./build-llvm.sh $PREFIX
	./build-lldb-mi.sh $PREFIX
	./install-wrappers.sh $PREFIX
	./build-mingw-w64.sh $PREFIX
	./build-mingw-w64-tools.sh $PREFIX
	./build-compiler-rt.sh $PREFIX
	./build-libcxx.sh $PREFIX
	./build-mingw-w64-libraries.sh $PREFIX
	./build-compiler-rt.sh $PREFIX --build-sanitizers
	./build-libssp.sh $PREFIX
	./build-openmp.sh $PREFIX
}

create_dir "${MY_OUTPUT_DIR}"

(cd "${LLVM_MINGW_DIR}"; prepare_sources)
(cd "${LLVM_MINGW_DIR}"; build "${MY_OUTPUT_DIR}")
