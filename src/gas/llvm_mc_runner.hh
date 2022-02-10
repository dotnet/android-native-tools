// SPDX-License-Identifier: MIT
#if !defined (__LLVM_MC_RUNNER_HH)
#define __LLVM_MC_RUNNER_HH

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "exceptions.hh"
#include "gas.hh"
#include "process.hh"

namespace xamarin::android::gas
{
	enum class LlvmMcArgument
	{
		Arch,
		FileType,
		GenerateDebug,
		IncludeDir,
		Mattr,
		Mcpu,
		Output,
	};

	enum class LlvmMcArchitecture
	{
		ARM64,
		ARM32,
		X86,
		X64,
	};

	class LlvmMcRunner
	{
	protected:
		// Value is `true` if the option can be set multiple times
		static std::unordered_map<LlvmMcArgument, bool> known_options;

	public:
		virtual ~LlvmMcRunner ()
		{}

		void set_input_file_path (fs::path const& file_path, bool derive_output_file_name = true)
		{
			if (file_path.empty ()) {
				throw invalid_argument_error { "Argument 'file_path' must not be empty" };
			}

			if (!file_path.has_filename ()) {
				throw invalid_argument_error { "Argument 'file_path' must have the file name portion" };
			}

			input_file_path = file_path;
			if (!derive_output_file_name) {
				return;
			}

			set_output_file_path (make_output_file_path (file_path));
		}

		fs::path make_output_file_path (fs::path const& input_file)
		{
			fs::path out_path = input_file;
			return out_path.replace_extension (".o").make_preferred ();
		}

		void set_output_file_path (fs::path const& file_path)
		{
			set_option (LlvmMcArgument::Output, file_path.string ());
		}

		void add_include_path (fs::path const& include_path)
		{
			if (include_path.empty ()) {
				return;
			}

			set_option (LlvmMcArgument::IncludeDir, include_path.string ());
		}

		void generate_debug_info ()
		{
			set_option (LlvmMcArgument::GenerateDebug);
		}

		virtual void map_option (std::string const& gas_name, std::string const& value = "") = 0;
		int run (fs::path const& executable_path);

	protected:
		LlvmMcRunner (LlvmMcArchitecture arch)
		{
			set_option (LlvmMcArgument::FileType, "obj");

			std::string architecture;
			std::string triple_prefix;
			switch (arch) {
				case LlvmMcArchitecture::ARM64:
					triple_prefix = architecture = "aarch64";
					break;

				case LlvmMcArchitecture::ARM32:
					triple_prefix = architecture = "arm";
					break;

				case LlvmMcArchitecture::X86:
					triple_prefix = architecture = "x86";
					break;

				case LlvmMcArchitecture::X64:
					architecture = "x86-64";
					triple_prefix = "x86_64";
					break;
			}

			set_arch (architecture);
			triple = triple_prefix + "-linux-gnu";
		}

		void set_arch (std::string const& arch)
		{
			set_option (LlvmMcArgument::Arch, arch);
		}

		void set_mcpu (std::string const& cpu)
		{
			set_option (LlvmMcArgument::Mcpu, cpu);
		}

		void set_option (LlvmMcArgument argument, std::string const& value = "")
		{
			if (argument == LlvmMcArgument::Arch) {
				if (arguments.find (argument) != arguments.end ()) {
					throw invalid_operation_error { "Architecture can be set only once" };
				}
			}

			bool is_multi = get_option_desc (argument);

			if (!is_multi) {
				arguments[argument] = value;
				return;
			}

			auto iter = arguments.find (argument);
			if (iter != arguments.end ()) {
				std::get<Process::string_list> (arguments[argument]).push_back (value);
			} else {
				arguments[argument] = Process::string_list { value };
			}
		}

		bool get_option_desc (LlvmMcArgument argument)
		{
			auto iter = known_options.find (argument);
			if (iter == known_options.end ()) {
				throw invalid_operation_error { "Unknown option" };
			}

			return iter->second;
		}

		void append_attributes (std::vector<std::string> const& new_attrs)
		{
			for (std::string const& attr : new_attrs) {
				set_option (LlvmMcArgument::Mattr, attr);
			}
		}

	private:
		std::unordered_map<LlvmMcArgument, Process::process_argument> arguments;
		fs::path input_file_path;
		std::string triple;
	};

	class LlvmMcRunnerARM64 final : public LlvmMcRunner
	{
	public:
		LlvmMcRunnerARM64 ()
			: LlvmMcRunner (LlvmMcArchitecture::ARM64)
		{}

		virtual ~LlvmMcRunnerARM64 ()
		{}

		virtual void map_option (std::string const& gas_name, std::string const& value = "") override final;
	};

	class LlvmMcRunnerARM32 final : public LlvmMcRunner
	{
	public:
		LlvmMcRunnerARM32 ()
			: LlvmMcRunner (LlvmMcArchitecture::ARM32)
		{}

		virtual ~LlvmMcRunnerARM32 ()
		{}

		virtual void map_option (std::string const& gas_name, std::string const& value = "") override final;

	private:
		static std::unordered_map<std::string, std::vector<std::string>> fpu_type_map;
	};

	class LlvmMcRunnerX64 final : public LlvmMcRunner
	{
	public:
		LlvmMcRunnerX64 ()
			: LlvmMcRunner (LlvmMcArchitecture::X64)
		{}

		virtual ~LlvmMcRunnerX64 ()
		{}

		virtual void map_option (std::string const& gas_name, std::string const& value = "") override final;
	};

	class LlvmMcRunnerX86 final : public LlvmMcRunner
	{
	public:
		LlvmMcRunnerX86 ()
			: LlvmMcRunner (LlvmMcArchitecture::X86)
		{}

		virtual ~LlvmMcRunnerX86 ()
		{}

		virtual void map_option (std::string const& gas_name, std::string const& value = "") override final;
	};
}
#endif // __LLVM_MC_RUNNER_HH
