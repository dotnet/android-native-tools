#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

PACKAGE_TREE_DIR="${ARTIFACTS_DIR}/package"
PACKAGE_ARTIFACTS_DIR="${PACKAGE_TREE_DIR}/artifacts"
PACKAGE_NAME_BASE="xamarin-android-llvm"

LLVM_VERSION=""

function prepare()
{
	local os="${1}"
	local artifacts_source="${ARTIFACTS_DIR}/${os}"
	local artifacts_dest="${PACKAGE_ARTIFACTS_DIR}/${os}"
	local version_file

	echo
	echo Preparing binaries for ${os}
	echo

	create_dir "${artifacts_dest}"
	local exe

	if [ "${os}" == "windows" ]; then
		exe=".exe"
	fi

	if [ -z "${LLVM_VERSION}" ]; then
		version_file="${artifacts_source}/llvm-version.txt"
		LLVM_VERSION=$(head -1 "${version_file}" | tr -d ' \n\t')
		cp "${version_file}" "${PACKAGE_ARTIFACTS_DIR}"

	fi

	for b in ${BINARIES}; do
		b="${b}${exe}"
		if [ "${os}" == "darwin" ]; then
			cp "${artifacts_source}/${b}.arm64" "${artifacts_dest}/${b}.arm64"
			cp "${artifacts_source}/${b}.x86_64" "${artifacts_dest}/${b}.x86_64"
		else
			cp "${artifacts_source}/${b}.upx" "${artifacts_dest}/${b}"
		fi
	done
}

for os in ${OPERATING_SYSTEMS}; do
	prepare ${os}
done

if [ -z "${LLVM_VERSION}" ]; then
	die Unable to detect LLVM version from the artifacts
fi

echo Creating package for LLVM version ${LLVM_VERSION}
(cd "${PACKAGE_TREE_DIR}"; tar cjf "${ARTIFACTS_DIR}/${PACKAGE_NAME_BASE}.tar.bz2" artifacts)
