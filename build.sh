#!/bin/bash -e
MY_NAME=$(basename "$0")
TRUE_PATH=$(readlink "$0" || echo "$0")
MY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_DIR="${MY_DIR}/build"
ARTIFACTS_DIR="${MY_DIR}/artifacts"
PATCHES_DIR="${MY_DIR}/patches"
VERSION_INFO_FILE="${ARTIFACTS_DIR}/version.txt"
CACHE_DIR="${HOME}/android-archives"
OS=$(uname -s)

ARCHITECTURES="arm arm64 x86 x86_64"
BASE_URL="https://ftp.gnu.org/gnu/binutils"
GETTER="$(which wget || which curl)"
MAKE="$(which gmake || which make)"
MACOS_TARGET="10.9"

VERSION="${1}"
BINUTILS_DIR_NAME="binutils-${VERSION}"
BINUTILS_PATCH_NAME="binutils-${VERSION}.diff"

function die()
{
	echo "$@"
	exit 1
}

function usage()
{
	cat <<EOF
Usage: ${MY_NAME} BINUTILS_VERSION
EOF
}

function arch_to_triple()
{
	local arch="${1}"

	if [ -z "${arch}" ]; then
		die arch_to_triple requires architecture name
	fi

	case "${arch}" in
		arm) triple="arm-linux-androideabi" ;;
		arm64) triple="aarch64-linux-android" ;;
		x86) triple="i686-linux-android" ;;
		x86_64) triple="x86_64-linux-android" ;;
		*) die Unsupported architecture ${arch} ;;
	esac

	echo "${triple}"
}

function get_build_tree()
{
	local host="${1}"
	local arch="${2}"

	echo "${BUILD_DIR}/${host}-${arch}"
}

function copy_artifact()
{
	local host="${1}"
	local arch="${2}"
	local source_file="${3}"
	local dest_file="${4}"

	if [ -z "${dest_file}" ]; then
		dest_file="$(basename "${source_file}")"
	fi

	local source_dir="$(get_build_tree "${host}" "${arch}")"
	local dest_dir="${ARTIFACTS_DIR}/${host}"
	local triple=$(arch_to_triple "${arch}")

	install -d -m 755 "${dest_dir}"
	cp "${source_dir}/${source_file}" "${dest_dir}/${triple}-${dest_file}"
}

function gather_artifacts()
{
	local host="${1}"
	local subdir="${2:-.}"
	local extension="${3}"
	local artifacts_dir="${ARTIFACTS_DIR}/${host}"

	install -d -m 755 "${artifacts_dir}"
	for arch in ${ARCHITECTURES}; do
		copy_artifact "${host}" "${arch}" "binutils/${subdir}/strip-new${extension}" "strip${extension}"
		copy_artifact "${host}" "${arch}" "gas/${subdir}/as-new${extension}" "as${extension}"
		copy_artifact "${host}" "${arch}" "ld/${subdir}/ld-new${extension}" "ld${extension}"
	done
}

function get_xcode_dir()
{
	local xcode_dir="$(xcode-select -p | head -1)"
	if [ $? -ne 0 ]; then
		die Could not find Xcode
	fi

	echo ${xcode_dir}
}

function configure_mac_compilers()
{
	if [ "${OS}" != "Darwin" ]; then
		return
	fi

	local xcode_dir="$(get_xcode_dir)"
	export CC="${xcode_dir}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
	export CXX="${xcode_dir}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
}

function detect_mac_arch_flags()
{
	if [ "${OS}" != "Darwin" ]; then
		return
	fi

	local xcode_dir="$(get_xcode_dir)"
	local sdksettings_path="${xcode_dir}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/SDKSettings.plist"
	local architectures="$(plutil -extract SupportedTargets.macosx.Archs json -o - "${sdksettings_path}")"

	if [ $? -ne 0 ]; then
		die Could not obtain the list of supported architectures from Xcode
	fi

	local flags=""

	if echo ${architectures} | grep '"arm64"' > /dev/null 2>&1; then
		flags="-arch arm64"
	fi

	if echo ${architectures} | grep '"x86_64"' > /dev/null 2>&1; then
		flags="${flags} -arch x86_64"
	fi

	local sysroot=""$(xcrun --sdk macosx --show-sdk-path)""

	#
	# The `-isysroot` and `-isystem` flags are required for autoconf to detect system headers when cross-compiling (which
	# technically is the case in the presence of multiple `-arch` arguments)
	#
	# Even though the search paths are set up correctly, autoconf fails to detect `string.h` when checking if all ANSI C
	# headers are defined (even though it detects it shortly after, "standalone"), so we need to define `STDC_HEADERS` here.
	# Similarly, `fcntl.h` is not detected (only in libiberty), so we force it here.
	#
	echo -n "${flags} -mmacosx-version-min=${MACOS_TARGET} -isysroot ${sysroot} -isystem ${sysroot} -DSTDC_HEADERS=1 -DHAVE_FCNTL_H"
}

function build()
{
	local host="${1}"

	if [ -z "${host}" ]; then
		die build requires host system name
	fi

	local arch="${2}"

	if [ -z "${arch}" ]; then
		die build requires architecture name
	fi

	tar xf "${TARBALL_DEST}"

	local source_dir="${BUILD_DIR}/${BINUTILS_DIR_NAME}"
	local binutils_patch="${PATCHES_DIR}/${BINUTILS_PATCH_NAME}"
	if [ -f "${binutils_patch}" ]; then
		(cd "${source_dir}"; patch -p1 < "${binutils_patch}")
	fi

	local configure_host

	case "${host}" in
		linux) configure_host="--host=x86_64-linux-gnu" ;;
		darwin) ;;
		windows) configure_host="--host=x86_64-w64-mingw32" ;;
		*) die Unsupported host ${host} ;;
	esac

	local triple=$(arch_to_triple "${arch}")
	local build_tree="$(get_build_tree "${host}" "${arch}")"
	install -d -m 755 "${build_tree}"

	local cflags="-O2 -m64 $(detect_mac_arch_flags)"
	local enable_gold
	local other_flags

	if [ "${arch}" == "arm64" ]; then
		other_flags="--enable-fix-cortex-a53-835769"
	fi

	configure_mac_compilers
	cd "${build_tree}"
	CFLAGS="${cflags}" CXXFLAGS="${cflags}" LDFLAGS="${cflags}" "${source_dir}/configure" \
		--target=${triple} \
		${configure_host} \
		--enable-initfini-array \
		--enable-64-bit-bfd \
		--disable-werror \
		--disable-nls \
		--disable-debug \
		--with-static-standard-libraries \
		${enable_gold} \
		${other_flags}

	"${MAKE}" -j
}

if [ -z "${VERSION}" ]; then
	usage
	die
fi

if [ -z "${GETTER}" ]; then
	die Could not find neither wget nor curl
fi

TARBALL_NAME="binutils-${VERSION}.tar.xz"
TARBALL_URL="${BASE_URL}/${TARBALL_NAME}"
TARBALL_DEST="${CACHE_DIR}/${TARBALL_NAME}"

if [ -d "${BUILD_DIR}" ]; then
	rm -rf "${BUILD_DIR}"
fi

if [ -d "${ARTIFACTS_DIR}" ]; then
	rm -rf "${ARTIFACTS_DIR}"
fi

install -d -m 755 "${BUILD_DIR}"

if [ ! -f "${TARBALL_DEST}" ]; then
	if [ ! -d "${CACHE_DIR}" ]; then
		install -d -m 755 "${CACHE_DIR}"
	fi

	case "$(basename "${GETTER}")" in
		wget) wget --progress=bar -O "${TARBALL_DEST}" "${TARBALL_URL}" ;;
		curl) curl -o "${TARBALL_DEST}" "${TARBALL_URL}" ;;
	esac
fi

HOSTS=""
if [ "${OS}" == "Linux" ]; then
	HOSTS="linux windows"
elif [ "${OS}" == "Darwin" ]; then
	HOSTS="darwin"
else
	die Unsupported build OS ${OS}
fi

for host in ${HOSTS}; do
	for arch in ${ARCHITECTURES}; do
		(cd "${BUILD_DIR}"; build ${host} ${arch})
		if [ $? -ne 0 ]; then
		   die Build ${host} ${arch} failed
		fi
	done
	if [ "${host}" == "windows" ]; then
		subdir=".libs"
		extension=".exe"
	else
		subdir=""
		extension=""
	fi
	gather_artifacts "${host}" "${subdir}" "${extension}"
done

echo "${VERSION}" > "${VERSION_INFO_FILE}"
