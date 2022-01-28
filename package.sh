#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

source common.sh

PACKAGE_TREE_DIR="${ARTIFACTS_DIR}/package"
PACKAGE_ARTIFACTS_DIR="${PACKAGE_TREE_DIR}/artifacts"

LLVM_VERSION=""

function make_windows_wrapper_scripts()
{
	local input="${1}"
	local output_dir="${2}"
	local output_base_name="${3}"
	local output_path
	local output_name

	for TRIPLE in ${ANDROID_TRIPLES}; do
		output_name="${TRIPLE}-${output_base_name}"
		output_path="${output_dir}/${output_name}.cmd"
		sed -e "s/@TARGET_NAME@/${output_name}/g" < "${input}" > "${output_path}"
	done
}

function make_unix_symlinks()
{
	local input="${1}"
	local output_dir="${2}"
	local output_base_name="${3}"
	local output_path
	local output_name

	for TRIPLE in ${ANDROID_TRIPLES}; do
		output_name="${TRIPLE}-${output_base_name}"
		output_path="${output_dir}/${output_name}"

		ln -sf "${input}" "${output_path}"
	done
}

function prepare()
{
	local os="${1}"
	local artifacts_source="${ARTIFACTS_DIR}/${os}"
	local artifacts_dest="${PACKAGE_ARTIFACTS_DIR}/${os}"
	local version_file

	echo
	echo Preparing binaries for ${os}
	echo

	create_empty_dir "${artifacts_dest}"
	local exe
	local cmd

	if [ "${os}" == "windows" ]; then
		exe=".exe"
		cmd=".cmd"

		make_windows_wrapper_scripts "scripts/llvm-strip.cmd.in" "${artifacts_source}" "strip"
		make_windows_wrapper_scripts "scripts/gas.cmd.in" "${artifacts_source}" "as"
	else
		make_unix_symlinks "llvm-strip" "${artifacts_source}" "strip"
		make_unix_symlinks "as" "${artifacts_source}" "as"
	fi

	if [ -z "${LLVM_VERSION}" ]; then
		version_file="${artifacts_source}/llvm-version.txt"
		LLVM_VERSION=$(head -1 "${version_file}" | tr -d ' \n\t')
		cp "${version_file}" "${PACKAGE_ARTIFACTS_DIR}"

	fi

	echo "Binaries: ${BINARIES}"
	set -x
	for b in ${BINARIES}; do
		if [ -f "${artifacts_source}/${b}${exe}" ]; then
			b="${b}${exe}"
		elif [ -f "${artifacts_source}/${b}${cmd}" ]; then
			b="${b}${cmd}"
		fi

		if [ -f "${artifacts_source}/${b}.upx" ]; then
			cp -P -a "${artifacts_source}/${b}.upx" "${artifacts_dest}/${b}"
		else
			cp -P -a "${artifacts_source}/${b}" "${artifacts_dest}/${b}"
		fi
	done
	set +x
}

for os in ${OPERATING_SYSTEMS}; do
	prepare ${os}
done

echo "${BINUTILS_VERSION}" > "${PACKAGE_ARTIFACTS_DIR}/binutils-version.txt"

if [ -z "${LLVM_VERSION}" ]; then
	die Unable to detect LLVM version from the artifacts
fi

echo Creating package for LLVM version ${LLVM_VERSION} and Binutils version ${BINUTILS_VERSION}
(cd "${PACKAGE_TREE_DIR}"; tar cjf "${ARTIFACTS_DIR}/${DIST_PACKAGE_NAME_BASE}.tar.bz2" artifacts)
