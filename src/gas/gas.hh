// SPDX-License-Identifier: MIT
#if !defined (__GAS_HH)
#define __GAS_HH

#include <unistd.h>
#include <getopt.h>
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

		static constexpr char arm64_arch_prefix[] = "aarch64-linux-android-";
		static constexpr char arm32_arch_prefix[] = "arm-linux-androideabi-";
		static constexpr char x86_arch_prefix[] = "i686-linux-android-";
		static constexpr char x64_arch_prefix[] = "x86_64-linux-android-";
#if defined (_WIN32)
		static constexpr char llvm_mc_name[] = "llvm-mc.exe";
#else
		static constexpr char llvm_mc_name[] = "llvm-mc";
#endif

	public:
		Gas ();

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
		void init_platform ();
		void determine_program_name (int argc, char **argv);
		int usage (bool is_error, std::string const message = "");

		std::string make_program_name (std::string const& arch_prefix)
		{
			std::string ret { arch_prefix };
			return ret.append (generic_program_name);
		}

	private:
		static std::vector<option> common_options;
		static std::vector<option> x86_options;
		static std::vector<option> arm32_options;

		std::vector<fs::path> input_files;

		std::string         _program_name;
		fs::path            _gas_output_file;
		fs::path            _program_dir;
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
