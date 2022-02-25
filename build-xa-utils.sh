#!/bin/bash -e

MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"
CONFIGURATION="${1:-Release}"

source ${MY_DIR}/common.sh

MY_BUILD_DIR="${BUILD_DIR}/xa-utils"
HOST_BIN_DIR="${MY_BUILD_DIR}/bin"
SOURCE_DIR="${MY_DIR}/src"

function configure()
{
	local cflags="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
	export CFLAGS="${cflags}"
	export CXXFLAGS="${cflags}"

	set -x
	cmake -G Ninja \
		  -DCMAKE_BUILD_TYPE="${CONFIGURATION}" \
		  -DBINUTILS_VERSION="2.38" \
		  -DLLVM_VERSION="${LLVM_VERSION}" \
		  "$@" \
		  "${SOURCE_DIR}"
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

function configure_windows()
{
	configure -DCMAKE_SYSTEM_NAME="Windows" \
			  -DCMAKE_CROSSCOMPILING="True" \
			  -DCMAKE_C_COMPILER="x86_64-w64-mingw32-gcc" \
			  -DCMAKE_CXX_COMPILER="x86_64-w64-mingw32-g++" \
			  -DCMAKE_RC_COMPILER="x86_64-w64-mingw32-windres" \
			  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM="NEVER" \
			  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY="ONLY" \
			  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE="ONLY"
}

function build()
{
	local host="${1}"

	ninja -j${JOBS}

	local exe
	local cmd
	if [ "${host}" == "windows" ]; then
		exe=".exe"
		cmd=".cmd"
	fi

	for b in ${XA_UTILS_BINARIES}; do
		cp -P -a "${HOST_BIN_DIR}/${b}${exe}" "${HOST_ARTIFACTS_BIN_DIR}/${b}${exe}"

		if [ "${host}" != "windows" ]; then
			strip "${HOST_ARTIFACTS_BIN_DIR}/${b}"
		fi

		if [ "${host}" == "linux" ]; then
			compress_binary "${HOST_ARTIFACTS_BIN_DIR}/${b}"
		fi
	done
}

create_empty_dir "${MY_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_BIN_DIR}"

case "${HOST}" in
	linux) JOBS=$(nproc) ;;
	darwin) JOBS=$(sysctl hw.ncpu | cut -d ':' -f 2 | tr -d ' ') ;;
	*) JOBS=1 ;;
esac

(cd "${MY_BUILD_DIR}"; configure_${HOST})
(cd "${MY_BUILD_DIR}"; build "${HOST}")

if [ "${HOST}" != "linux" ]; then
	exit 0
fi

MY_BUILD_DIR="${MY_BUILD_DIR}-windows"
HOST_ARTIFACTS_DIR="${ARTIFACTS_DIR}/windows"
HOST_ARTIFACTS_BIN_DIR="${HOST_ARTIFACTS_DIR}/bin"
HOST_BIN_DIR="${MY_BUILD_DIR}/bin"

create_empty_dir "${MY_BUILD_DIR}"
create_dir "${HOST_ARTIFACTS_BIN_DIR}"

(cd "${MY_BUILD_DIR}"; configure_windows)
(cd "${MY_BUILD_DIR}"; build windows)
