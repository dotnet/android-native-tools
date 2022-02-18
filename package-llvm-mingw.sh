#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

MINGW_ARTIFACTS_DIR="llvm-mingw"
PACKAGE_PATH="${ARTIFACTS_DIR}/${MINGW_DIST_PACKAGE_NAME_BASE}.tar.bz2"

if [ ! -f "${MINGW_VERSION_INFO_PATH}" ]; then
	die Missing MinGW version info file ${MINGW_VERSION_INFO_PATH}
fi
LLVM_VER=$(get_nth_line 1 | cut -d ':' -f 2)
MINGW_VER=$(get_nth_line 2 | cut -d ':' -f 2)

echo "Creating LLVM (${LLVM_VER}) + MinGW (${MINGW_VER}) package"
if [ -f "${PACKAGE_PATH}" ]; then
	rm "${PACKAGE_PATH}"
fi
tar cjf "${PACKAGE_PATH}" artifacts/${MINGW_ARTIFACTS_DIR}
