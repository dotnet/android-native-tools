#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

WORK_DIR="${MY_DIR}/prep"
TARBALL_BASE_NAME="xamarin-android-binutils"
ARTIFACT_TARBALL="${TARBALL_BASE_NAME}.tar.bz2"
ARTIFACTS_DIR="${WORK_DIR}/artifacts"

ARTIFACT_ZIP="${1}"
TAG_NAME="${2}"

function die()
{
	echo "$@"
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${MY_NAME} ARTIFACT_ZIP TAG_NAME

where

  ARTIFACT_ZIP is a path to the artifact produced by the commit you want to release.
               The file must be downloaded manually from the build artifacts area.
  TAG_NAME is a new tag for the upcoming release
EOF
	exit 0
}

function prepare()
{
	unzip "${ARTIFACT_ZIP}"

	if [ ! -f "${ARTIFACT_TARBALL}" ]; then
		die Build artifact tarball $(pwd)/${ARTIFACT_TARBALL} not found
	fi

	tar xf "${ARTIFACT_TARBALL}"
	if [ ! -d "${ARTIFACTS_DIR}" ]; then
		die Artifacts directory ${ARTIFACTS_DIR} does not exist
	fi

	echo "${TAG_NAME}" >> "${ARTIFACTS_DIR}/version.txt"

	local dest_archive="${MY_DIR}/${TARBALL_BASE_NAME}-${TAG_NAME}.7z"

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
  * Create new release using the ${TAG_NAME} tag
  * Upload ${dest_archive} to that release
EOF
}

if [ -z "${ARTIFACT_ZIP}" -o -z "${TAG_NAME}" ]; then
	usage
fi

if [ ! -f "${ARTIFACT_ZIP}" ]; then
	die Artifact ZIP archive not found at ${ARTIFACT_ZIP}
fi

git fetch -t

if git tag -l | grep "${TAG_NAME}" > /dev/null 2>&1; then
	die Tag ${TAG_NAME} already exists, please choose a new one
fi

if [ -d "${WORK_DIR}" ]; then
	rm -rf "${WORK_DIR}"
fi
install -d -m 755 "${WORK_DIR}"

ARTIFACT_ZIP="$(cd "$(dirname ${ARTIFACT_ZIP})"; pwd)/"$(basename ${ARTIFACT_ZIP})""

(cd "${WORK_DIR}"; prepare)
