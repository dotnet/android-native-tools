#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

PACKAGE_TREE_DIR="${ARTIFACTS_DIR}/package"
PACKAGE_ARTIFACTS_DIR="${PACKAGE_TREE_DIR}/artifacts"
PACKAGE_NAME_BASE="xamarin-android-llvm"

OPERATING_SYSTEMS="linux darwin" # windows
LLVM_VERSION=""

function prepare()
{
	local os="${1}"
	local artifacts_source="${ARTIFACTS_DIR}/${os}"
	local artifacts_dest="${PACKAGE_ARTIFACTS_DIR}/${os}"

	echo
	echo Preparing binaries for ${os}
	echo

	if [ -z "${LLVM_VERSION}" ]; then
		LLVM_VERSION=$(head -1 "${artifacts_source}/llvm-version.txt" | tr -d ' \n\t')
	fi
	create_dir "${artifacts_dest}"
	for b in ${BINARIES}; do
		cp "${artifacts_source}/${b}" "${artifacts_dest}/${b}"

		# We could use --best, but it's way too slow for not enough gain
		upx -9 "${artifacts_dest}/${b}"
		upx -t "${artifacts_dest}/${b}"
	done
}

for os in ${OPERATING_SYSTEMS}; do
	prepare ${os}
done

if [ -z "${LLVM_VERSION}" ]; then
	die Unable to detect LLVM version from the artifacts
fi

(cd "${PACKAGE_TREE_DIR}"; tar cjf "${ARTIFACTS_DIR}/${PACKAGE_NAME_BASE}-${LLVM_VERSION}.tar.bz2" artifacts)
