#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source common.sh

WORK_DIR="${MY_DIR}/prep"
ARTIFACT_TARBALL="${DIST_PACKAGE_NAME_BASE}.tar.bz2"
ARTIFACTS_DIR="${WORK_DIR}/artifacts"

ARTIFACT_ZIP="${1}"
XA_TAG_COMPONENT="${2}"

function die()
{
	echo "$@"
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${MY_NAME} ARTIFACT_ZIP XA_TAG_COMPONENT

where

  ARTIFACT_ZIP is a path to the artifact produced by the commit you want to release.
               The file must be downloaded manually from the build artifacts area.
  XA_TAG_COMPONENT will be combined with LLVM versionin the following manner: {LLVM_VERSION}-{XA_TAG_COMPONENT}
  and the resulting string will be used as the new tag name
EOF
	exit 0
}

function prepare()
{
	echo Unpacking artifact ZIP
	unzip "${ARTIFACT_ZIP}"

	if [ ! -f "${ARTIFACT_TARBALL}" ]; then
		die Build artifact tarball $(pwd)/${ARTIFACT_TARBALL} not found
	fi

	echo Unpacking binaries tarball
	tar xf "${ARTIFACT_TARBALL}"
	if [ ! -d "${ARTIFACTS_DIR}" ]; then
		die Artifacts directory ${ARTIFACTS_DIR} does not exist
	fi

	local llvm_version=$(head -1 "${ARTIFACTS_DIR}/llvm-version.txt" | tr -d ' \t')
	local tag_name="L_${llvm_version}-B_${BINUTILS_VERSION}-${XA_TAG_COMPONENT}"
	echo "${tag_name}" >> "${ARTIFACTS_DIR}/version.txt"

	echo New build for LLVM ${llvm_version} found
	echo New tag name: ${tag_name}

	git fetch -t

	if git tag -l | grep "${tag_name}" > /dev/null 2>&1; then
		die Tag ${tag_name} already exists, please choose a new one
	fi

	local dest_archive="${MY_DIR}/${DIST_PACKAGE_NAME_BASE}-${tag_name}.7z"

	if [ -f "${dest_archive}" ]; then
		rm "${dest_archive}"
	fi

	(cd "${ARTIFACTS_DIR}"; 7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "${dest_archive}" .)

	echo
	echo Release archive: ${dest_archive}
	echo
	cat <<EOF
Next steps:

  * Go to https://github.com/xamarin/xamarin-android-binutils/releases
  * Create new release using the '${tag_name}' tag
  * Upload ${dest_archive} to that release
EOF
}

if [ -z "${ARTIFACT_ZIP}" -o -z "${XA_TAG_COMPONENT}" ]; then
	usage
fi

if [ ! -f "${ARTIFACT_ZIP}" ]; then
	die Artifact ZIP archive not found at ${ARTIFACT_ZIP}
fi

if [ -d "${WORK_DIR}" ]; then
	rm -rf "${WORK_DIR}"
fi
install -d -m 755 "${WORK_DIR}"

ARTIFACT_ZIP="$(cd "$(dirname ${ARTIFACT_ZIP})"; pwd)/"$(basename ${ARTIFACT_ZIP})""

(cd "${WORK_DIR}"; prepare)
