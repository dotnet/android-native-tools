#!/bin/bash -e

MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source ${MY_DIR}/common.sh

PROJECTS="lld"
TARGETS="X86;ARM;AArch64"

MY_BUILD_DIR="${BUILD_DIR}/llvm"
HOST_BIN_DIR="${MY_BUILD_DIR}/bin"
HOST_LIB_DIR="${MY_BUILD_DIR}/lib"
LLVM_VERSION_FILE="${HOST_ARTIFACTS_DIR}/llvm-version.txt"

SOURCE_DIR="${MY_DIR}/external/llvm/llvm"

function configure()
{
	local cflags="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
	export CFLAGS="${cflags}"
	export CXXFLAGS="${cflags}"

	set -x
	cmake -G Ninja \
		  -DBUILD_SHARED_LIBS=ON \
		  -DCMAKE_BUILD_TYPE=Release \
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
              -DCMAKE_OSX_DEPLOYMENT_TARGET="${MACOS_TARGET}" \
              -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
}

function copy_libs()
{
	local extension="${1}"

	cp -P -a "${HOST_LIB_DIR}"/lib*.${extension} "${HOST_ARTIFACTS_LIB_DIR}"

	#
	# Don't strip on macOS, it will end up exiting with this error
	#
	#   /Applications/Xcode_13.2.1.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip: error: symbols referenced by indirect symbol table entries
        #    that can't be stripped in: /Users/runner/work/xamarin-android-binutils/xamarin-android-binutils/artifacts/darwin/lib/liblldMinGW.dylib (for architecture x86_64)
	#
	# Followed by a list of quite a few symbols
	#
	if [ "${HOST}" == "linux" ]; then
		strip "${HOST_ARTIFACTS_LIB_DIR}/"lib*.${extension}
	fi
}

function build()
{
	ninja -j${JOBS} llvm-objcopy
	ninja -j${JOBS} llvm-mc
	ninja -j${JOBS} lld
	ninja -j${JOBS} llc

	mv "${HOST_BIN_DIR}/llvm-objcopy" "${HOST_BIN_DIR}/llvm-strip"
	grep 'CMAKE_PROJECT_VERSION:' "${MY_BUILD_DIR}/CMakeCache.txt" | cut -d '=' -f 2 > "${LLVM_VERSION_FILE}"

	for b in ${LLVM_BINARIES}; do
		cp -P -a "${HOST_BIN_DIR}/${b}" "${HOST_ARTIFACTS_BIN_DIR}/${b}"
		strip "${HOST_ARTIFACTS_BIN_DIR}/${b}"
	done

	if [ "${HOST}" == "linux" ]; then
		copy_libs "so.*"
	else
		copy_libs "dylib"
	fi
}

create_empty_dir "${MY_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_BIN_DIR}"
create_dir "${HOST_ARTIFACTS_LIB_DIR}"

case "${HOST}" in
	linux) JOBS=$(nproc) ;;
	darwin) JOBS=$(sysctl hw.ncpu | cut -d ':' -f 2 | tr -d ' ') ;;
	*) JOBS=1 ;;
esac

(cd "${MY_BUILD_DIR}"; configure_${HOST})
(cd "${MY_BUILD_DIR}"; build)
