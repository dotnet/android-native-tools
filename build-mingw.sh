#!/bin/bash -e

MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

REAL_HOST="${HOST}"
HOST=windows
PROJECTS="llvm-mc llvm-objcopy"
TARGETS="X86;ARM;AArch64"
TOOLS="llvm-mc llvm-objcopy"

HOST_BUILD_DIR="${BUILD_DIR}/${HOST}"
HOST_BIN_DIR="${HOST_BUILD_DIR}/bin"
HOST_ARTIFACTS_DIR="${ARTIFACTS_DIR}/${HOST}"
LLVM_VERSION_FILE="${HOST_ARTIFACTS_DIR}/llvm-version.txt"

SOURCE_DIR="${MY_DIR}/external/llvm/llvm"

function configure()
{
	local cflags="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
	export CFLAGS="${cflags}"
	export CXXFLAGS="${cflags}"

	set -x
	cmake -G Ninja \
		  -DBUILD_SHARED_LIBS=OFF \
		  -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_CROSSCOMPILING=ON \
		  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
		  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
		  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
		  -DCMAKE_SYSTEM_NAME=Windows \
		  -DCROSS_TOOLCHAIN_FLAGS_LLVM_NATIVE="-DLLVM_BUILD_BENCHMARKS=OFF;-DLLVM_INCLUDE_BENCHMARKS=OFF;-DCMAKE_CROSSCOMPILING=ON;-DLLVM_INCLUDE_TESTS=OFF" \
		  -DLLVM_BUILD_BENCHMARKS=OFF \
		  -DLLVM_BUILD_DOCS=OFF \
		  -DLLVM_BUILD_EXAMPLES=OFF \
		  -DLLVM_BUILD_RUNTIMES=OFF \
		  -DLLVM_BUILD_TESTS=OFF \
		  -DLLVM_BUILD_TOOLS=OFF \
		  -DLLVM_CCACHE_BUILD=ON \
		  -DLLVM_ENABLE_ASSERTIONS=OFF \
		  -DLLVM_ENABLE_BINDINGS=OFF \
		  -DLLVM_ENABLE_FFI=OFF \
		  -DLLVM_ENABLE_IDE=OFF \
		  -DLLVM_ENABLE_LIBEDIT=OFF \
		  -DLLVM_ENABLE_LIBPFM=OFF \
		  -DLLVM_ENABLE_LIBXML2=OFF \
		  -DLLVM_ENABLE_PROJECTS="${PROJECTS}" \
		  -DLLVM_ENABLE_TERMINFO=OFF \
		  -DLLVM_ENABLE_THREADS=OFF \
		  -DLLVM_ENABLE_ZLIB=OFF \
		  -DLLVM_INCLUDE_BENCHMARKS=OFF \
		  -DLLVM_INCLUDE_EXAMPLES=OFF \
		  -DLLVM_INCLUDE_TESTS=OFF \
		  -DLLVM_INCLUDE_TOOLS=ON \
		  -DLLVM_OPTIMIZED_TABLEGEN=ON \
		  -DLLVM_STATIC_LINK_CXX_STDLIB=ON \
		  -DLLVM_TARGETS_TO_BUILD="${TARGETS}" \
		  "${SOURCE_DIR}"
	set +x
}

function build()
{
	ninja -j${JOBS} llvm-objcopy
	ninja -j${JOBS} llvm-mc

	grep 'CMAKE_PROJECT_VERSION:' "${HOST_BUILD_DIR}/CMakeCache.txt" | cut -d '=' -f 2 > "${LLVM_VERSION_FILE}"

	for b in ${BINARIES}; do
		cp "${HOST_BIN_DIR}/${b}" "${HOST_ARTIFACTS_DIR}/${b}"
		strip "${HOST_ARTIFACTS_DIR}/${b}"
	done
}

create_dir "${HOST_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_DIR}"

case "${REAL_HOST}" in
	linux) JOBS=$(nproc) ;;
	darwin) JOBS=$(sysctl hw.ncpu | cut -d ':' -f 2 | tr -d ' ') ;;
	*) JOBS=1 ;;
esac

(cd "${HOST_BUILD_DIR}"; configure)
(cd "${HOST_BUILD_DIR}"; build)
