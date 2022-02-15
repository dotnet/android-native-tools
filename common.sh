#!/bin/bash -e

XA_UTILS_VERSION="4.0.0"
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
	sed -n \
		-e 's/^.*set.*(LLVM_VERSION_MAJOR \(.*\))/\1/p' \
		-e 's/^.*set.*(LLVM_VERSION_MINOR \(.*\))/\1/p' \
		-e 's/^.*set.*(LLVM_VERSION_PATCH \(.*\))/\1/p' \
		external/llvm/llvm/CMakeLists.txt | xargs echo -n | tr ' ' '.'
}

HOST=$(uname | tr A-Z a-z)
ANDROID_TRIPLES="aarch64-linux-android arm-linux-androideabi i686-linux-android x86_64-linux-android"
BUILD_DIR="${MY_DIR}/xa-build/${HOST}"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
HOST_ARTIFACTS_DIR="${ARTIFACTS_DIR}/${HOST}"
LLVM_BINARIES="llvm-mc llvm-strip lld"
LLVM_PREFIXED_BINARIES="$(make_prefixed_binaries strip) $(make_prefixed_binaries ld)"
XA_UTILS_BINARIES="as"
XA_UTILS_PREFIXED_BINARIES="$(make_prefixed_binaries as)"
BINARIES="${LLVM_BINARIES} ${LLVM_PREFIXED_BINARIES} ${XA_UTILS_BINARIES} ${XA_UTILS_PREFIXED_BINARIES}"
OPERATING_SYSTEMS="linux darwin windows"
DIST_PACKAGE_NAME_BASE="xamarin-android-toolchain"
LLVM_VERSION="$(detect_llvm_version)"
