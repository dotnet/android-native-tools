#!/bin/bash -e

BINUTILS_VERSION="2.37"
MACOS_TARGET="10.12"

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

HOST=$(uname | tr A-Z a-z)
BUILD_DIR="${MY_DIR}/xa-build/${HOST}"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
HOST_ARTIFACTS_DIR="${ARTIFACTS_DIR}/${HOST}"
LLVM_BINARIES="llvm-mc llvm-objcopy"
ANDROID_TRIPLES="aarch64-linux-android arm-linux-androideabi i686-linux-android x86_64-linux-android"
BINUTILS_BINARIES="$(make_prefixed_binaries ld)"
XATU_BINARIES="as"
BINARIES="${BINUTILS_BINARIES} ${LLVM_BINARIES}"
OPERATING_SYSTEMS="linux darwin windows"
DIST_PACKAGE_NAME_BASE="xamarin-android-toolchain"
