// SPDX-License-Identifier: MIT
#if !defined (__GAS_HH)
#define __GAS_HH

#include <unistd.h>
#include <getopt.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace xamarin::android::gas
{
	namespace fs = std::filesystem;

	class LlvmMcRunner;

	enum class TargetArchitecture
	{
		ARM32,
		ARM64,
		X86,
		X64,
	};

	enum OptionValues
	{
		OPTION_IGNORE = 100,
		OPTION_O,
		OPTION_WARN,
		OPTION_G,
		OPTION_MFPU,
		OPTION_VERSION,
		OPTION_VERSION_EXIT,
		OPTION_HELP,
	};

	template<size_t Size>
	using char_array = std::array<char, Size>;

	template<size_t ...Length>
	static constexpr auto concat_const (const char (&...parts)[Length])
	{
		// `parts` being constant string arrays, Length for each of them includes the trailing NUL byte, thus the
		// `sizeof... (Length)` part which subtracts the number of template parameters - the amount of NUL bytes so that
		// we don't waste space.
		constexpr size_t total_length = (... + Length) - sizeof... (Length);
		char_array<total_length + 1> ret;
		ret[total_length] = 0;

		size_t i = 0;
		for (char const* from : {parts...}) {
			for (; *from != '\0'; i++) {
				ret[i] = *from++;
			}
		}

		return ret;
	}

	class Gas final
	{
		struct ParseArgsResult
		{
			const bool terminate;
			const bool is_error;

			ParseArgsResult (bool _terminate, bool _is_error) noexcept
				: terminate (_terminate),
				  is_error (_is_error)
			{}
		};

		static constexpr char generic_gas_name[] =
#if defined (_WIN32)
			"as.exe"
#else
			"as"
#endif
			;

		static constexpr char generic_ld_name[] =
#if defined (_WIN32)
			"ld.exe"
#else
			"ld"
#endif
			;

		static constexpr char arm64_arch_prefix[] = "aarch64-linux-android-";
		static constexpr char arm32_arch_prefix[] = "arm-linux-androideabi-";
		static constexpr char x86_arch_prefix[] = "i686-linux-android-";
		static constexpr char x64_arch_prefix[] = "x86_64-linux-android-";

	public:
		~Gas ()
		{}

		int run (int argc, char **argv);

		const std::string& program_name () const noexcept
		{
			return _program_name;
		}

		const fs::path& program_dir () const noexcept
		{
			return _program_dir;
		}

		TargetArchitecture target_arch () const noexcept
		{
			return _target_arch;
		}

	protected:
		ParseArgsResult parse_arguments (int argc, char **argv, std::unique_ptr<LlvmMcRunner>& mc_runner);

	private:
		void determine_program_name (int argc, char **argv);
		int usage (bool is_error, std::string const message = "");

	private:
		static constexpr auto arm64_gas_name = concat_const (arm64_arch_prefix, generic_gas_name);
		static constexpr auto arm32_gas_name = concat_const (arm32_arch_prefix, generic_gas_name);
		static constexpr auto x86_gas_name   = concat_const (x86_arch_prefix, generic_gas_name);
		static constexpr auto x64_gas_name   = concat_const (x64_arch_prefix, generic_gas_name);

		static constexpr auto arm64_ld_name  = concat_const (arm64_arch_prefix, generic_ld_name);
		static constexpr auto arm32_ld_name  = concat_const (arm32_arch_prefix, generic_ld_name);
		static constexpr auto x86_ld_name    = concat_const (x86_arch_prefix, generic_ld_name);
		static constexpr auto x64_ld_name    = concat_const (x64_arch_prefix, generic_ld_name);

		static std::vector<option> common_options;
		static std::vector<option> x86_options;
		static std::vector<option> arm32_options;

		std::vector<fs::path> input_files;

		std::string         _program_name;
		fs::path            _gas_output_file;
		fs::path            _program_dir;
		TargetArchitecture  _target_arch;
	};
}

extern xamarin::android::gas::Gas app;
#endif // __GAS_HH
