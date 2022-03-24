#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source ${MY_DIR}/common.sh

MY_OUTPUT_DIR="${MINGW_BASE_OUTPUT_DIR}/${HOST}"
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

	CMAKEFLAGS="-DLLDB_ENABLE_PYTHON=OFF" ./build-llvm.sh --disable-lldb $PREFIX
	./build-lldb-mi.sh $PREFIX
	./install-wrappers.sh $PREFIX
	DEFAULT_MSVCRT=msvcrt ./build-mingw-w64.sh $PREFIX
	./build-mingw-w64-tools.sh $PREFIX
	./build-compiler-rt.sh $PREFIX
	./build-libcxx.sh $PREFIX
	./build-mingw-w64-libraries.sh $PREFIX
	./build-compiler-rt.sh $PREFIX --build-sanitizers
	./build-libssp.sh $PREFIX
	./build-openmp.sh $PREFIX
}

function gather_versions()
{
	local mingw_ver=$(sed -n -E -e 's/^AC_INIT\(\[.*\],\[(.*)\],.*\)/\1/p' < mingw-w64/configure.ac)
	local llvm_ver=$(detect_llvm_version llvm-project)

	cat <<EOF > "${MINGW_VERSION_INFO_PATH}"
L:${llvm_ver}
M:${mingw_ver}
EOF
}

create_dir "${MY_OUTPUT_DIR}"

(cd "${LLVM_MINGW_DIR}"; prepare_sources)
(cd "${LLVM_MINGW_DIR}"; build "${MY_OUTPUT_DIR}")
(cd "${LLVM_MINGW_DIR}"; gather_versions)
