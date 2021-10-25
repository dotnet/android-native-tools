#!/bin/bash -e

MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"
HOST=$(uname | tr A-Z a-z)

PROJECTS="lld"
TARGETS="X86;ARM;AArch64"
TOOLS="lld llvm-mc llvm-objcopy"

BUILD_DIR="${MY_DIR}/xa-build"
HOST_BUILD_DIR="${BUILD_DIR}/${HOST}"
HOST_BIN_DIR="${HOST_BUILD_DIR}/bin"
HOST_ARTIFACTS_DIR="${MY_DIR}/artifacts/${HOST}"
LLVM_VERSION_FILE="${HOST_ARTIFACTS_DIR}/llvm-version.txt"

SOURCE_DIR="${MY_DIR}/external/llvm/llvm"

BINARIES="lld llvm-mc llvm-objcopy"

function die()
{
	echo "$*"
	exit 1
}

function configure()
{
	local cflags="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
	export CFLAGS="${cflags}"
	export CXXFLAGS="${cflags}"

	set -x
	cmake -G Ninja \
		  -DCMAKE_BUILD_TYPE=Release \
		  -DBUILD_SHARED_LIBS=OFF \
		  -DLLVM_ENABLE_PROJECTS="${PROJECTS}" \
		  -DLLVM_TARGETS_TO_BUILD="${TARGETS}" \
		  -DLLVM_BUILD_BENCHMARKS=OFF \
		  -DLLVM_INCLUDE_BENCHMARKS=OFF \
		  -DLLVM_BUILD_DOCS=OFF \
		  -DLLVM_BUILD_EXAMPLES=OFF \
		  -DLLVM_INCLUDE_EXAMPLES=OFF \
		  -DLLVM_BUILD_TESTS=OFF \
		  -DLLVM_INCLUDE_TESTS=OFF \
		  -DLLVM_BUILD_TOOLS=OFF \
		  -DLLVM_INCLUDE_TOOLS=ON \
		  -DLLVM_CCACHE_BUILD=ON \
		  -DLLVM_ENABLE_BINDINGS=OFF \
		  -DLLVM_ENABLE_FFI=OFF \
		  -DLLVM_ENABLE_IDE=OFF \
		  -DLLVM_ENABLE_ZLIB=OFF \
		  -DLLVM_ENABLE_LIBXML2=OFF \
		  -DLLVM_STATIC_LINK_CXX_STDLIB=ON \
		  -DLLVM_ENABLE_TERMINFO=OFF \
		  -DLLVM_ENABLE_LIBEDIT=OFF \
		  -DLLVM_ENABLE_LIBPFM=OFF \
		  -DLLVM_ENABLE_ASSERTIONS=OFF \
		  -DLLVM_BUILD_RUNTIMES=OFF \
		  -DLLVM_OPTIMIZED_TABLEGEN=ON \
		  -DLLVM_ENABLE_THREADS=OFF \
		  "${SOURCE_DIR}"
	set +x
}

function build()
{
	ninja -j${JOBS}
	ninja -j${JOBS} llvm-mc

	grep 'CMAKE_PROJECT_VERSION:' "${HOST_BUILD_DIR}/CMakeCache.txt" | cut -d '=' -f 2 > "${LLVM_VERSION_FILE}"

	for b in ${BINARIES}; do
		cp "${HOST_BIN_DIR}/${b}" "${HOST_ARTIFACTS_DIR}/${b}"
		strip "${HOST_ARTIFACTS_DIR}/${b}"
	done

	# for t in ${TOOLS}; do
	# 	upx -9 -obin/${t}.upx bin/${t}
	# done
}

function create_dir()
{
	local dir="${1}"

	if [ -d "${dir}" ]; then
		rm -rf "${dir}"
	fi
	install -d -m 755 "${dir}"
}

create_dir "${HOST_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_DIR}"

case "${HOST}" in
	linux) JOBS=$(($(nproc) - 2)) ;;
	darwin) JOBS=$(($(sysctl hw.ncpu | cut -d ':' -f 2 | tr -d ' ') - 2)) ;;
	*) JOBS=1 ;;
esac

(cd "${HOST_BUILD_DIR}"; configure)
(cd "${HOST_BUILD_DIR}"; build)
