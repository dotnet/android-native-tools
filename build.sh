#!/bin/bash -e

MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

PROJECTS="lld"
TARGETS="X86;ARM;AArch64"
TOOLS="lld llvm-mc llvm-objcopy"

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
		  "$@" \
		  "${SOURCE_DIR}"
	set +x
}

function configure_linux()
{
	configure
}

function configure_darwin()
{
	configure -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
              -DCMAKE_OSX_DEPLOYMENT_TARGET='10.12' \
              -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
}

function compress_binary()
{
	local input="${1}"

	upx -9 -o "${input}.upx" "${input}"
	upx -t "${input}.upx"
}

function build()
{
	ninja -j${JOBS}
	ninja -j${JOBS} llvm-mc

	grep 'CMAKE_PROJECT_VERSION:' "${HOST_BUILD_DIR}/CMakeCache.txt" | cut -d '=' -f 2 > "${LLVM_VERSION_FILE}"

	for b in ${BINARIES}; do
		cp "${HOST_BIN_DIR}/${b}" "${HOST_ARTIFACTS_DIR}/${b}"
		strip "${HOST_ARTIFACTS_DIR}/${b}"
		if [ "${HOST}" == "darwin" ]; then
			lipo -extract x86_64 -output "${HOST_ARTIFACTS_DIR}/${b}.x86_64" "${HOST_ARTIFACTS_DIR}/${b}"
			lipo -extract arm64 -output "${HOST_ARTIFACTS_DIR}/${b}.arm64" "${HOST_ARTIFACTS_DIR}/${b}"
			rm "${HOST_ARTIFACTS_DIR}/${b}"
			compress_binary "${HOST_ARTIFACTS_DIR}/${b}.x86_64"
			compress_binary "${HOST_ARTIFACTS_DIR}/${b}.arm64"
		else
			compress_binary "${HOST_ARTIFACTS_DIR}/${b}"
		fi
	done
}

create_dir "${HOST_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_DIR}"

case "${HOST}" in
	linux) JOBS=$(nproc) ;;
	darwin) JOBS=$(sysctl hw.ncpu | cut -d ':' -f 2 | tr -d ' ') ;;
	*) JOBS=1 ;;
esac

(cd "${HOST_BUILD_DIR}"; configure_${HOST})
(cd "${HOST_BUILD_DIR}"; build)
