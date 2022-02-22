#!/bin/bash -e

XA_UTILS_VERSION="5.0.0"

#
# Note: LLVM MinGW requires at least 10.13.4
#
MACOS_TARGET="10.15"

if [ -z "${MY_DIR}" ]; then
	echo common.sh must be included after defining the MY_DIR variable
	exit 1
fi

function die()
{
	echo "$*"
	exit 1
}

function create_dir()
{
	local dir="${1}"

	install -d -m 755 "${dir}"
}

function create_empty_dir()
{
	local dir="${1}"

	if [ -d "${dir}" ]; then
		rm -rf "${dir}"
	fi

	create_dir "${dir}"
}

function compress_binary()
{
	local input="${1}"
	local output="${input}.upx"

	if [ -f "${output}" ]; then
		rm "${output}"
	fi

	upx -9 -o "${output}" "${input}"
	upx -t "${output}"
}

function make_prefixed_binaries()
{
	local binary="${1}"

	for triple in ${ANDROID_TRIPLES}; do
		echo -n "${triple}-${binary} "
	done
}

function detect_llvm_version()
{
	local llvm_top="${1}"

	sed -n \
		-e 's/^.*set.*(LLVM_VERSION_MAJOR \(.*\))/\1/p' \
		-e 's/^.*set.*(LLVM_VERSION_MINOR \(.*\))/\1/p' \
		-e 's/^.*set.*(LLVM_VERSION_PATCH \(.*\))/\1/p' \
		"${llvm_top}/llvm/CMakeLists.txt" | xargs echo -n | tr ' ' '.'
}

function get_nth_line()
{
	local line="${1}"

	echo $(head -n ${line} "${MINGW_VERSION_INFO_PATH}" | tail -n 1)
}

HOST=$(uname | tr A-Z a-z)
ANDROID_TRIPLES="aarch64-linux-android arm-linux-androideabi i686-linux-android x86_64-linux-android"
BUILD_DIR="${MY_DIR}/xa-build/${HOST}"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
HOST_ARTIFACTS_DIR="${ARTIFACTS_DIR}/${HOST}"
HOST_ARTIFACTS_BIN_DIR="${HOST_ARTIFACTS_DIR}/bin"
HOST_ARTIFACTS_LIB_DIR="${HOST_ARTIFACTS_DIR}/lib"
LLVM_BINARIES="llvm-mc llvm-strip lld llc"
LLVM_PREFIXED_BINARIES="$(make_prefixed_binaries strip) $(make_prefixed_binaries ld)"
XA_UTILS_BINARIES="as"
XA_UTILS_PREFIXED_BINARIES="$(make_prefixed_binaries as)"
BINARIES="${LLVM_BINARIES} ${LLVM_PREFIXED_BINARIES} ${XA_UTILS_BINARIES} ${XA_UTILS_PREFIXED_BINARIES}"
OPERATING_SYSTEMS="linux darwin windows"
XAT_DIST_PACKAGE_NAME_BASE="xamarin-android-toolchain"
MINGW_DIST_PACKAGE_NAME_BASE="llvm-mingw-toolchain"
RELEASE_PACKAGE_NAME_BASE="${XAT_DIST_PACKAGE_NAME_BASE}"
LLVM_VERSION="$(detect_llvm_version "${MY_DIR}/external/llvm")"
PACKAGE_TREE_DIR="${ARTIFACTS_DIR}/package"
PACKAGE_ARTIFACTS_DIR="${PACKAGE_TREE_DIR}/artifacts"
MINGW_BASE_OUTPUT_DIR="${ARTIFACTS_DIR}/llvm-mingw"
MINGW_VERSION_INFO_PATH="${MINGW_BASE_OUTPUT_DIR}/version.txt"
