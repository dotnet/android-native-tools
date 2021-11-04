// SPDX-License-Identifier: MIT
#if !defined (__GAS_HH)
#define __GAS_HH

#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

namespace xamarin::android::gas
{
	enum class TargetArchitecture
	{
		ARM32,
		ARM64,
		X86,
		X64,
	};

	class Gas final
	{
		static constexpr int OPTION_STD_BASE = 150;

		enum OptionValues
		{
			OPTION_HELP = OPTION_STD_BASE,
			OPTION_NOCPP,
			OPTION_STATISTICS,
			OPTION_VERSION,
			OPTION_DUMPCONFIG,
			OPTION_VERBOSE,
			OPTION_EMULATION,
			OPTION_DEBUG_PREFIX_MAP,
			OPTION_DEFSYM,
			OPTION_LISTING_LHS_WIDTH,
			OPTION_LISTING_LHS_WIDTH2,
			OPTION_LISTING_RHS_WIDTH,
			OPTION_LISTING_CONT_LINES,
			OPTION_DEPFILE,
			OPTION_GSTABS,
			OPTION_GSTABS_PLUS,
			OPTION_GDWARF_2,
			OPTION_GDWARF_3,
			OPTION_GDWARF_4,
			OPTION_GDWARF_5,
			OPTION_GDWARF_SECTIONS,
			OPTION_GDWARF_CIE_VERSION,
			OPTION_STRIP_LOCAL_ABSOLUTE,
			OPTION_TRADITIONAL_FORMAT,
			OPTION_WARN,
			OPTION_TARGET_HELP,
			OPTION_EXECSTACK,
			OPTION_NOEXECSTACK,
			OPTION_SIZE_CHECK,
			OPTION_ELF_STT_COMMON,
			OPTION_ELF_BUILD_NOTES,
			OPTION_SECTNAME_SUBST,
			OPTION_ALTERNATE,
			OPTION_AL,
			OPTION_HASH_TABLE_SIZE,
			OPTION_REDUCE_MEMORY_OVERHEADS,
			OPTION_WARN_FATAL,
			OPTION_COMPRESS_DEBUG,
			OPTION_NOCOMPRESS_DEBUG,
			OPTION_NO_PAD_SECTIONS,

			// amd64 and arm32 options
			OPTION_EB,
			OPTION_EL,

			// arm32 options
			OPTION_FIX_V4BX,
			OPTION_FDPIC,

			// x86 and x64 options
			OPTION_32,
			OPTION_64,
			OPTION_DIVIDE,
			OPTION_MARCH,
			OPTION_MTUNE,
			OPTION_MMNEMONIC,
			OPTION_MSYNTAX,
			OPTION_MINDEX_REG,
			OPTION_MNAKED_REG,
			OPTION_MRELAX_RELOCATIONS,
			OPTION_MSSE2AVX,
			OPTION_MSSE_CHECK,
			OPTION_MOPERAND_CHECK,
			OPTION_MAVXSCALAR,
			OPTION_X32,
			OPTION_MADD_BND_PREFIX,
			OPTION_MEVEXLIG,
			OPTION_MEVEXWIG,
			OPTION_MBIG_OBJ,
			OPTION_MOMIT_LOCK_PREFIX,
			OPTION_MEVEXRCIG,
			OPTION_MSHARED,
			OPTION_MAMD64,
			OPTION_MINTEL64,
			OPTION_MFENCE_AS_LOCK_ADD,
			OPTION_X86_USED_NOTE,
			OPTION_MVEXWIG,
			OPTION_MALIGN_BRANCH_BOUNDARY,
			OPTION_MALIGN_BRANCH_PREFIX_SIZE,
			OPTION_MALIGN_BRANCH,
			OPTION_MBRANCHES_WITH_32B_BOUNDARIES,
			OPTION_MLFENCE_AFTER_LOAD,
			OPTION_MLFENCE_BEFORE_INDIRECT_BRANCH,
			OPTION_MLFENCE_BEFORE_RET,
			OPTION_MUSE_UNALIGNED_VECTOR_MOVE,
		};

		static constexpr char arm64_arch_prefix[] = "aarch64-linux-android-";
		static constexpr char arm32_arch_prefix[] = "arm-linux-androideabi-";
		static constexpr char x86_arch_prefix[] = "i686-linux-android-";
		static constexpr char x64_arch_prefix[] = "x86_64-linux-android-";

		static const std::vector<option> common_long_options;
		static const std::vector<char> common_short_options;

		static const std::vector<option> arm64_long_options;
		static const std::vector<char> arm64_short_options;

		static const std::vector<option> arm32_long_options;
		static const std::vector<char> arm32_short_options;

		static const std::vector<option> x86_long_options;
		static const std::vector<char> x86_short_options;

	public:
		Gas ();

		int run (int argc, char **argv);

		const char* program_name () const noexcept
		{
			return _program_name;
		}

		TargetArchitecture target_arch () const noexcept
		{
			return _target_arch;
		}

	protected:
		bool parse_arguments (int argc, char **argv);

	private:
		void init_platform ();
		void determine_program_name (int argc, char **argv);
		int usage (bool is_error, std::string const message);

		std::string make_program_name (std::string const& arch_prefix)
		{
			std::string ret { arch_prefix };
			return ret.append (generic_program_name);
		}

	private:
		char               *_program_name;
		TargetArchitecture  _target_arch;

		std::string         generic_program_name { "as" };
		std::string         arm64_program_name;
		std::string         arm32_program_name;
		std::string         x86_program_name;
		std::string         x64_program_name;
		std::string         newline;
	};
}

extern xamarin::android::gas::Gas app;
#endif // __GAS_HH
