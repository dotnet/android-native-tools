// SPDX-License-Identifier: MIT
#if !defined (__LLVM_MC_RUNNER_HH)
#define __LLVM_MC_RUNNER_HH

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include "gas.hh"

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

	struct LlvmMcOption
	{
		const std::string name;
		const bool is_long;
		const bool is_path;
		const bool is_multi;
		const bool has_argument;
		const bool uses_comma_separated_list;

		explicit LlvmMcOption (std::string const& name, bool is_long, bool is_path, bool is_multi, bool has_argument, bool uses_comma_separated_list = false)
			: name (name),
			  is_long (is_long),
			  is_path (is_path),
			  is_multi (is_multi),
			  has_argument (has_argument),
			  uses_comma_separated_list (uses_comma_separated_list)
		{}
	};

	class LlvmMcRunner
	{
		using string_list = std::vector<std::string>;
		using process_argument = std::variant<std::string, string_list>;

	protected:
		static std::unordered_map<LlvmMcArgument, LlvmMcOption> known_options;

	public:
		virtual ~LlvmMcRunner ()
		{}

		void set_input_file_path (std::filesystem::path const& file_path, bool derive_output_file_name = true)
		{
			if (file_path.empty ()) {
				return; // TODO: throw instead
			}

			if (!file_path.has_filename ()) {
				return; // TODO: throw instead
			}

			input_file_path = file_path;
			if (!derive_output_file_name) {
				return;
			}

			set_output_file_path (make_output_file_path (file_path));
		}

		void set_output_file_path (std::filesystem::path const& file_path)
		{
			set_option (LlvmMcArgument::Output, file_path.string ());
		}

		void add_include_path (std::filesystem::path const& include_path)
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
			switch (arch) {
				case LlvmMcArchitecture::ARM64:
					architecture = "aarch64";
					break;

				case LlvmMcArchitecture::ARM32:
					architecture = "arm";
					break;

				case LlvmMcArchitecture::X86:
					architecture = "x86";
					break;

				case LlvmMcArchitecture::X64:
					architecture = "x86-64";
					break;
			}

			set_arch (architecture);
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
					throw "TODO: Replace with real exception (Arch can be set only once)";
				}
			}

			LlvmMcOption const& desc = get_option_desc (argument);

			if (!desc.is_multi) {
				arguments[argument] = value;
				return;
			}

			auto iter = arguments.find (argument);
			if (iter != arguments.end ()) {
				std::get<string_list> (arguments[argument]).push_back (value);
			} else {
				arguments[argument] = string_list { value };
			}
		}

		std::filesystem::path make_output_file_path (std::filesystem::path const& input_file)
		{
			std::filesystem::path out_path = input_file;
			return out_path.replace_extension (".o").make_preferred ();
		}

		LlvmMcOption const& get_option_desc (LlvmMcArgument argument)
		{
			auto iter = known_options.find (argument);
			if (iter == known_options.end ()) {
				throw "TODO: replace with a real exception (unknown option)";
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
		void append_program_argument (std::vector<std::string>& args, std::string const& option_name, std::string const& option_value = "");
		void append_program_argument (std::vector<std::string>& args, std::string const& option_name, string_list const& option_value, bool uses_comma_separated_list = false);

		void append_program_argument (std::vector<std::string>& args, std::string const& option_name, process_argument const& option_value, bool uses_comma_separated_list = false)
		{
			switch (option_value.index ()) {
				case 0:
					append_program_argument (args, option_name, std::get<std::string> (option_value));
					break;

				case 1:
					append_program_argument (args, option_name, std::get<string_list> (option_value), uses_comma_separated_list);
					break;
			}
		}

	private:
		std::unordered_map<LlvmMcArgument, process_argument> arguments;
		std::filesystem::path input_file_path;
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
