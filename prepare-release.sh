#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source common.sh

WORK_DIR="${MY_DIR}/prep"
ACTIONS_URL="https://github.com/xamarin/xamarin-android-binutils/actions"
XAT_ARTIFACT_TARBALL="${XAT_DIST_PACKAGE_NAME_BASE}.tar.bz2"
MINGW_ARTIFACT_TARBALL="${MINGW_DIST_PACKAGE_NAME_BASE}.tar.bz2"
ARTIFACTS_DIR="${WORK_DIR}/artifacts"

XAT_ARTIFACT_ZIP="Xamarin.Android.zip"
MINGW_ARTIFACT_ZIP="llvm-mingw.zip"

XA_TAG_COMPONENT="${1}"

function die()
{
	echo "$@"
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${MY_NAME} XA_TAG_COMPONENT

where
  XA_TAG_COMPONENT will be combined with LLVM versionin the following manner:

    L1_{XA_LLVM_VERSION}-L2_{MINGW_LLVM_VERSION}-M_{MINGW_VERSION}-{XA_TAG_COMPONENT}

  and the resulting string will be used as the new tag name.
  The L1 component represents version of LLVM used to build the Xamarin.Android toolchain,
  while the L2 component represents version of LLVM used to build MinGW.

  Prior to running this script both ${XAT_ARTIFACT_ZIP} and ${MINGW_ARTIFACT_ZIP} archives
  must be downloaded from the appropriate run (see ${ACTIONS_URL})
  and placed in the same director as the ${MY_NAME} script.
EOF
	exit 0
}

function die_no_artifact_zip()
{
	local artifact_zip="${1}"

	echo Artifact ZIP archive not found at ${MY_DIR}${artifact_zip}
	die Please download it from ${ACTIONS_URL}
}

function ensure_artifact_zip()
{
	local artifact_zip="${1}"

	if [ ! -f "${artifact_zip}" ]; then
		die_no_artifact_zip ${artifact_zip}
	fi
}

function prepare()
{
	echo Unpacking Xamarin.Android toolchain artifact ZIP
	unzip "${MY_DIR}/${XAT_ARTIFACT_ZIP}"

	echo Unpacking LLVM MinGW toolchain artifact ZIP
	unzip "${MY_DIR}/${MINGW_ARTIFACT_ZIP}"

	if [ ! -f "${XAT_ARTIFACT_TARBALL}" ]; then
		die Xamarin.Android toolchain build artifact tarball $(pwd)/${XAT_ARTIFACT_TARBALL} not found
	fi

	if [ ! -f "${MINGW_ARTIFACT_TARBALL}" ]; then
		die LLVM MinGW toolchain build artifact tarball $(pwd)/${MINGW_ARTIFACT_TARBALL} not found
	fi

	echo Unpacking Xamarin.Android toolchain tarball
	tar xf "${XAT_ARTIFACT_TARBALL}"

	echo Unpacking LLVM MinGW toolchain tarball
	tar xf "${MINGW_ARTIFACT_TARBALL}"

	if [ ! -d "${ARTIFACTS_DIR}" ]; then
		die Artifacts directory ${ARTIFACTS_DIR} does not exist
	fi

	local xa_llvm_version=$(head -1 "${ARTIFACTS_DIR}/llvm-version.txt" | tr -d ' \t')
	local mingw_llvm_version=$(get_nth_line 1 | cut -d ':' -f 2)
	local mingw_version=$(get_nth_line 2 | cut -d ':' -f 2)
	local tag_name="L1_${xa_llvm_version}-L2_${mingw_llvm_version}-M_${mingw_version}-${XA_TAG_COMPONENT}"
	echo "${tag_name}" >> "${ARTIFACTS_DIR}/version.txt"
	echo "X:${XA_TAG_COMPONENT}" >> "${ARTIFACTS_DIR}/llvm-mingw/version.txt"

	echo Xamarin.Android LLVM version: ${xat_llvm_version}
	echo MinGW LLVM version: ${mingw_llvm_version}
	echo MinGW version: ${mingw_version}
	echo
	echo New tag name: ${tag_name}

	git fetch -t

	if git tag -l | grep "${tag_name}" > /dev/null 2>&1; then
		die Tag ${tag_name} already exists, please choose a new one
	fi

	local dest_archive="${MY_DIR}/${RELEASE_PACKAGE_NAME_BASE}-${tag_name}.7z"

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

if [ -z "${XA_TAG_COMPONENT}" ]; then
	usage
fi

ensure_artifact_zip "${XAT_ARTIFACT_ZIP}"
ensure_artifact_zip "${MINGW_ARTIFACT_ZIP}"

if [ -d "${WORK_DIR}" ]; then
	rm -rf "${WORK_DIR}"
fi
install -d -m 755 "${WORK_DIR}"

(cd "${WORK_DIR}"; prepare)
