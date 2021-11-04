#!/bin/bash -e

BINUTILS_VERSION="2.37"
MACOS_TARGET="10.12"

if [ -z "${MY_DIR}" ]; then
	echo common.sh must be included after defining the MY_DIR variable
	exit 1
fi

HOST=$(uname | tr A-Z a-z)
BUILD_DIR="${MY_DIR}/xa-build/${HOST}"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
LLVM_BINARIES="llvm-mc llvm-objcopy"
BINUTILS_BINARIES="aarch64-linux-android-ld arm-linux-androideabi-ld i686-linux-android-ld x86_64-linux-android-ld"
BINARIES="${BINUTILS_BINARIES} ${LLVM_BINARIES}"
OPERATING_SYSTEMS="linux darwin windows"
DIST_PACKAGE_NAME_BASE="xamarin-android-toolchain"

function die()
{
	echo "$*"
	exit 1
}

function create_empty_dir()
{
	local dir="${1}"

	if [ -d "${dir}" ]; then
		rm -rf "${dir}"
	fi
	install -d -m 755 "${dir}"
}
