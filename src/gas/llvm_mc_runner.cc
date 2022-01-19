// SPDX-License-Identifier: MIT
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

std::unordered_map<LlvmMcArgument, LlvmMcOption> LlvmMcRunner::known_options {
	{
		LlvmMcArgument::Arch,
		LlvmMcOption {
			"arch",
			true /* is_long */,
			true /* is_path */,
			false /* is_multi */,
			true /* has_argument */
		}
	},

	{
		LlvmMcArgument::FileType,
		LlvmMcOption {
			"filetype",
			true /* is_long */,
			false /* is_path */,
			false /* is_multi */,
			true /* has_argument */
		}
	},

	{
		LlvmMcArgument::IncludeDir,
		LlvmMcOption {
			"I",
			false /* is_long */,
			true /* is_path */,
			true /* is_multi */,
			true /* has_argument */
		}
	},

	{
		LlvmMcArgument::Mcpu,
		LlvmMcOption {
			"mcpu",
			true /* is_long */,
			false /* is_path */,
			false /* is_multi */,
			true /* has_argument */
		}
	},

	{
		LlvmMcArgument::Output,
		LlvmMcOption {
			"o",
			false /* is_long */,
			true /* is_path */,
			false /* is_multi */,
			true /* has_argument */
		}
	},

	{
		LlvmMcArgument::Mattr,
		LlvmMcOption {
			"mattr",
			true /* is_long */,
			false /* is_path */,
			true /* is_multi */,
			true /* has_argument */,
			true /* uses_comma_separated_list */
		}
	},

	{
		LlvmMcArgument::GenerateDebug,
		LlvmMcOption {
			"g",
			false /* is_long */,
			false /* is_path */,
			false /* is_multi */,
			false /* has_argument */,
			false /* uses_comma_separated_list */
		}
	},
};

void LlvmMcRunner::append_program_argument (std::vector<std::string>& args, std::string const& option_name, std::string const& option_value)
{
	if (option_value.empty ()) {
		args.push_back (option_name);
		return;
	}

	std::string value_arg { option_name };
	value_arg.append ("=");
	value_arg.append (option_value);

	args.push_back (value_arg);
}

void LlvmMcRunner::append_program_argument (std::vector<std::string>& args, std::string const& option_name, string_list const& option_value, bool uses_comma_separated_list)
{
	if (option_value.empty ()) {
		append_program_argument (args, option_name);
		return;
	}

	if (!uses_comma_separated_list) {
		for (std::string const& value : option_value) {
			append_program_argument (args, option_name, value);
		}
		return;
	}

	std::string value_arg { option_name };
	value_arg.append ("=");

	std::string value_list;
	for (std::string const& value : option_value) {
		if (!value_list.empty ()) {
			value_list.append (",");
		}
		value_list.append (value);
	}
	value_arg.append (value_list);

	args.push_back (value_arg);
}

int LlvmMcRunner::run (fs::path const& executable_path)
{
	std::vector<std::string> args;

	auto opt = arguments.find (LlvmMcArgument::Arch);
	if (opt != arguments.end ()) {
		append_program_argument (args, "--arch", opt->second);
	}

	append_program_argument (args, "--assemble");

	opt = arguments.find (LlvmMcArgument::GenerateDebug);
	if (opt != arguments.end ()) {
		append_program_argument (args, "-g");
	}

	opt = arguments.find (LlvmMcArgument::FileType);
	if (opt != arguments.end ()) {
		append_program_argument (args, "--filetype", opt->second);
	}

	opt = arguments.find (LlvmMcArgument::Mattr);
	if (opt != arguments.end ()) {
		append_program_argument (args, "--mattr", opt->second, true /* uses_comma_separated_list */);
	}

	opt = arguments.find (LlvmMcArgument::Output);
	if (opt != arguments.end ()) {
		append_program_argument (args, "-o", opt->second);
	}

	args.push_back (input_file_path.make_preferred ().string ());

	std::vector<const char*> exec_args;

	for (std::string const& arg : args) {
		exec_args.push_back (arg.c_str ());
	}
	exec_args.push_back (nullptr);

	std::cout << "Would run: " << executable_path;
	for (const char* arg : exec_args) {
		if (arg == nullptr) {
			continue;
		}
		std::cout << " " << arg;
	}
	std::cout << "\n";

	return 0;
}
