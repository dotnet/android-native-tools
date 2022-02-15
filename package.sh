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

function make_unix_wrapper_scripts()
{
	local input="${1}"
	local output_dir="${2}"
	local output_base_name="${3}"
	local output_path
	local output_name

	for TRIPLE in ${ANDROID_TRIPLES}; do
		output_name="${TRIPLE}-${output_base_name}"
		output_path="${output_dir}/${output_name}"

		cp -a -P "${input}" "${output_path}"
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
		make_windows_wrapper_scripts "scripts/ld.cmd.in" "${artifacts_source}" "ld"
	else
		make_unix_wrapper_scripts "scripts/llvm-strip.sh" "${artifacts_source}" "strip"
		make_unix_wrapper_scripts "scripts/gas.sh" "${artifacts_source}" "as"
		make_unix_wrapper_scripts "scripts/ld.sh" "${artifacts_source}" "ld"
	fi

	if [ -z "${LLVM_VERSION}" ]; then
		version_file="${artifacts_source}/llvm-version.txt"
		LLVM_VERSION=$(head -1 "${version_file}" | tr -d ' \n\t')
		cp "${version_file}" "${PACKAGE_ARTIFACTS_DIR}"
	fi

	local dest_b=
	for b in ${BINARIES}; do
		if [ "${b}" == "lld" ]; then
			dest_b="ld"
		else
			dest_b="${b}"
		fi

		if [ -f "${artifacts_source}/${b}${exe}" ]; then
			b="${b}${exe}"
			dest_b="${dest_b}${exe}"
		elif [ -f "${artifacts_source}/${b}${cmd}" ]; then
			b="${b}${cmd}"
			dest_b="${dest_b}${cmd}"
		fi

		if [ -f "${artifacts_source}/${b}.upx" ]; then
			cp -P -a "${artifacts_source}/${b}.upx" "${artifacts_dest}/${dest_b}"
		else
			cp -P -a "${artifacts_source}/${b}" "${artifacts_dest}/${dest_b}"
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
(cd "${PACKAGE_TREE_DIR}"; tar cjf "${ARTIFACTS_DIR}/${DIST_PACKAGE_NAME_BASE}.tar.bz2" artifacts)
