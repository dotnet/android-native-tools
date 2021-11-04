// SPDX-License-Identifier: MIT
#include <unistd.h>
#include <iostream>

#include "gas.hh"

using namespace xamarin::android::gas;

Gas::Gas ()
{
	arm64_program_name = make_program_name (arm64_arch_prefix);
	arm32_program_name = make_program_name (arm32_arch_prefix);
	x86_program_name = make_program_name (x86_arch_prefix);
	x64_program_name = make_program_name (x64_arch_prefix);

	init_platform ();
}

int Gas::usage (bool is_error, std::string const message)
{
	if (!message.empty ()) {
		std::cerr << message << newline;
	}

	std::cerr << "`" << program_name () << "` takes the same arguments as the GNU Assembler (gas) program." << newline
	          << "Please read the `as(1)` manual page or visit https://sourceware.org/binutils/docs-" << BINUTILS_VERSION << "/as/Invoking.html#Invoking" << newline
	          << "Command line options are compatibile with GAS version " << BINUTILS_VERSION << newline
	          << newline;

	return is_error ? 1 : 0;
}

int Gas::run (int argc, char **argv)
{
	determine_program_name (argc, argv);
	std::cout << "Program name: " << program_name () << newline;

	if (arm64_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::ARM64;
	} else if (arm32_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::ARM32;
	} else if (x86_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::X86;
	} else if (x64_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::X64;
	} else if (generic_program_name.compare (program_name ()) == 0) {
		std::string message { "Program invoked via its generic name (" };
		message
			.append (generic_program_name)
			.append ("), please use one of the ABI-prefixed names:")
			.append (newline)
			.append ("  ").append (arm64_program_name).append (newline)
			.append ("  ").append (arm32_program_name).append (newline)
			.append ("  ").append (x86_program_name).append (newline)
			.append ("  ").append (x64_program_name).append (newline);
		return usage (true /* is_error */, message);
	} else {
		std::string message { "Unknown program name '" };
		message.append (program_name ()).append ("'").append (newline);
		return usage (true /* is_error */, message);
	}

	if (!parse_arguments (argc, argv)) {
		return 1;
	}

	return 0;
}

const std::vector<char> Gas::common_short_options {
	'-', 'J', 'K', 'L', 'M', 'R', 'W', 'Z', 'a', ':',
	':', 'D', 'f', 'g', ':',':', 'I', ':', 'o', ':',
	'v', 'w', 'X', 't', ':',
};

const std::vector<option> Gas::common_long_options {
	{"alternate",                    no_argument,       nullptr, OPTION_ALTERNATE},
	{"a",                            optional_argument, nullptr, 'a'},
	{"al",                           optional_argument, nullptr, OPTION_AL},
	{"compress-debug-sections",      optional_argument, nullptr, OPTION_COMPRESS_DEBUG},
	{"nocompress-debug-sections",    no_argument,       nullptr, OPTION_NOCOMPRESS_DEBUG},
	{"debug-prefix-map",             required_argument, nullptr, OPTION_DEBUG_PREFIX_MAP},
	{"defsym",                       required_argument, nullptr, OPTION_DEFSYM},
	{"dump-config",                  no_argument,       nullptr, OPTION_DUMPCONFIG},
	{"emulation",                    required_argument, nullptr, OPTION_EMULATION},
	{"execstack",                    no_argument,       nullptr, OPTION_EXECSTACK},
	{"noexecstack",                  no_argument,       nullptr, OPTION_NOEXECSTACK},
	{"size-check",                   required_argument, nullptr, OPTION_SIZE_CHECK},
	{"elf-stt-common",               required_argument, nullptr, OPTION_ELF_STT_COMMON},
	{"sectname-subst",               no_argument,       nullptr, OPTION_SECTNAME_SUBST},
	{"generate-missing-build-notes", required_argument, nullptr, OPTION_ELF_BUILD_NOTES},
	{"fatal-warnings",               no_argument,       nullptr, OPTION_WARN_FATAL},
	{"gdwarf-2",                     no_argument,       nullptr, OPTION_GDWARF_2},
	{"gdwarf-3",                     no_argument,       nullptr, OPTION_GDWARF_3},
	{"gdwarf-4",                     no_argument,       nullptr, OPTION_GDWARF_4},
	{"gdwarf-5",                     no_argument,       nullptr, OPTION_GDWARF_5},
	{"gdwarf2",                      no_argument,       nullptr, OPTION_GDWARF_2},
	{"gdwarf-sections",              no_argument,       nullptr, OPTION_GDWARF_SECTIONS},
	{"gdwarf-cie-version",           required_argument, nullptr, OPTION_GDWARF_CIE_VERSION},
	{"gen-debug",                    no_argument,       nullptr, 'g'},
	{"gstabs",                       no_argument,       nullptr, OPTION_GSTABS},
	{"gstabs+",                      no_argument,       nullptr, OPTION_GSTABS_PLUS},
	{"hash-size",                    required_argument, nullptr, OPTION_HASH_TABLE_SIZE},
	{"help",                         no_argument,       nullptr, OPTION_HELP},
	{"itbl",                         required_argument, nullptr, 't'},
	{"keep-locals",                  no_argument,       nullptr, 'L'},
	{"keep-locals",                  no_argument,       nullptr, 'L'},
	{"listing-lhs-width",            required_argument, nullptr, OPTION_LISTING_LHS_WIDTH},
	{"listing-lhs-width2",           required_argument, nullptr, OPTION_LISTING_LHS_WIDTH2},
	{"listing-rhs-width",            required_argument, nullptr, OPTION_LISTING_RHS_WIDTH},
	{"listing-cont-lines",           required_argument, nullptr, OPTION_LISTING_CONT_LINES},
	{"MD",                           required_argument, nullptr, OPTION_DEPFILE},
	{"mri",                          no_argument,       nullptr, 'M'},
	{"nocpp",                        no_argument,       nullptr, OPTION_NOCPP},
	{"no-pad-sections",              no_argument,       nullptr, OPTION_NO_PAD_SECTIONS},
	{"no-warn",                      no_argument,       nullptr, 'W'},
	{"reduce-memory-overheads",      no_argument,       nullptr, OPTION_REDUCE_MEMORY_OVERHEADS},
	{"statistics",                   no_argument,       nullptr, OPTION_STATISTICS},
	{"strip-local-absolute",         no_argument,       nullptr, OPTION_STRIP_LOCAL_ABSOLUTE},
	{"version",                      no_argument,       nullptr, OPTION_VERSION},
	{"verbose",                      no_argument,       nullptr, OPTION_VERBOSE},
	{"target-help",                  no_argument,       nullptr, OPTION_TARGET_HELP},
	{"traditional-format",           no_argument,       nullptr, OPTION_TRADITIONAL_FORMAT},
	{"warn",                         no_argument,       nullptr, OPTION_WARN},
};

const std::vector<char> Gas::arm64_short_options {
	'm', ':',
};

const std::vector<option> Gas::arm64_long_options {
	{"EB", no_argument, nullptr, OPTION_EB},
	{"EL", no_argument, nullptr, OPTION_EL},
};

const std::vector<char> Gas::arm32_short_options {
	'm', ':', 'k',
};

const std::vector<option> Gas::arm32_long_options {
	{"EB",       no_argument, nullptr, OPTION_EB},
	{"EL",       no_argument, nullptr, OPTION_EL},
	{"fix-v4bx", no_argument, nullptr, OPTION_FIX_V4BX},
	{"fdpic",    no_argument, nullptr, OPTION_FDPIC},
};

const std::vector<char> Gas::x86_short_options {
	'k', 'V', 'Q', ':', 's', 'q', 'n', 'O', ':', ':'
};

const std::vector<option> Gas::x86_long_options {
	{"32",                              no_argument,       nullptr, OPTION_32},
	{"64",                              no_argument,       nullptr, OPTION_64},
	{"x32",                             no_argument,       nullptr, OPTION_X32},
	{"mshared",                         no_argument,       nullptr, OPTION_MSHARED},
	{"mx86-used-note",                  required_argument, nullptr, OPTION_X86_USED_NOTE},
	{"divide",                          no_argument,       nullptr, OPTION_DIVIDE},
	{"march",                           required_argument, nullptr, OPTION_MARCH},
	{"mtune",                           required_argument, nullptr, OPTION_MTUNE},
	{"mmnemonic",                       required_argument, nullptr, OPTION_MMNEMONIC},
	{"msyntax",                         required_argument, nullptr, OPTION_MSYNTAX},
	{"mindex-reg",                      no_argument,       nullptr, OPTION_MINDEX_REG},
	{"mnaked-reg",                      no_argument,       nullptr, OPTION_MNAKED_REG},
	{"msse2avx",                        no_argument,       nullptr, OPTION_MSSE2AVX},
	{"muse-unaligned-vector-move",      no_argument,       nullptr, OPTION_MUSE_UNALIGNED_VECTOR_MOVE},
	{"msse-check",                      required_argument, nullptr, OPTION_MSSE_CHECK},
	{"moperand-check",                  required_argument, nullptr, OPTION_MOPERAND_CHECK},
	{"mavxscalar",                      required_argument, nullptr, OPTION_MAVXSCALAR},
	{"mvexwig",                         required_argument, nullptr, OPTION_MVEXWIG},
	{"madd-bnd-prefix",                 no_argument,       nullptr, OPTION_MADD_BND_PREFIX},
	{"mevexlig",                        required_argument, nullptr, OPTION_MEVEXLIG},
	{"mevexwig",                        required_argument, nullptr, OPTION_MEVEXWIG},
	{"momit-lock-prefix",               required_argument, nullptr, OPTION_MOMIT_LOCK_PREFIX},
	{"mfence-as-lock-add",              required_argument, nullptr, OPTION_MFENCE_AS_LOCK_ADD},
	{"mrelax-relocations",              required_argument, nullptr, OPTION_MRELAX_RELOCATIONS},
	{"mevexrcig",                       required_argument, nullptr, OPTION_MEVEXRCIG},
	{"malign-branch-boundary",          required_argument, nullptr, OPTION_MALIGN_BRANCH_BOUNDARY},
	{"malign-branch-prefix-size",       required_argument, nullptr, OPTION_MALIGN_BRANCH_PREFIX_SIZE},
	{"malign-branch",                   required_argument, nullptr, OPTION_MALIGN_BRANCH},
	{"mbranches-within-32B-boundaries", no_argument,       nullptr, OPTION_MBRANCHES_WITH_32B_BOUNDARIES},
	{"mlfence-after-load",              required_argument, nullptr, OPTION_MLFENCE_AFTER_LOAD},
	{"mlfence-before-indirect-branch",  required_argument, nullptr, OPTION_MLFENCE_BEFORE_INDIRECT_BRANCH},
	{"mlfence-before-ret",              required_argument, nullptr, OPTION_MLFENCE_BEFORE_RET},
	{"mamd64",                          no_argument,       nullptr, OPTION_MAMD64},
	{"mintel64",                        no_argument,       nullptr, OPTION_MINTEL64},
};

bool Gas::parse_arguments (int argc, char **argv)
{
	std::vector<char> short_options { common_short_options };
	std::vector<option> long_options { common_long_options };

	switch (target_arch ()) {
		case TargetArchitecture::ARM64:
			short_options.insert (short_options.end (), arm64_short_options.begin (), arm64_short_options.end ());
			long_options.insert (long_options.end (), arm64_long_options.begin (), arm64_long_options.end ());
			break;

		case TargetArchitecture::ARM32:
			short_options.insert (short_options.end (), arm32_short_options.begin (), arm32_short_options.end ());
			long_options.insert (long_options.end (), arm32_long_options.begin (), arm32_long_options.end ());
			break;

		case TargetArchitecture::X86:
		case TargetArchitecture::X64:
			short_options.insert (short_options.end (), x86_short_options.begin (), x86_short_options.end ());
			long_options.insert (long_options.end (), x86_long_options.begin (), x86_long_options.end ());
			break;
	}
	short_options.push_back ('\0'); // getopt will need to see it as a standard C string
	long_options.emplace_back ();

	while (true) {
		int long_index;
		int option_char = getopt_long_only (argc, argv, short_options.data (), long_options.data (), &long_index);

		switch (option_char) {
			case '?':
				usage (true /* is_error */, "Unrecognized option");
				return false;

			default:
				break;
		}
	}

	return true;
}
