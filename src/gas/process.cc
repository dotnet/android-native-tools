// SPDX-License-Identifier: MIT
#include "process.hh"

using namespace xamarin::android::gas;

void Process::print_process_command_line ()
{
	std::cout << "Running: " << executable_path;
	for (std::string const& arg : _args) {
		std::cout << " " << arg;
	}
	std::cout << "\n";
}

std::vector<std::string::const_pointer> Process::make_exec_args ()
{
	std::vector<std::string::const_pointer> exec_args;

	exec_args.push_back (executable_path.string ().c_str ());
	for (std::string const& arg : _args) {
		exec_args.push_back (arg.c_str ());
	}

	return exec_args;
}

void Process::append_program_argument (std::string const& option_name, std::string const& option_value)
{
	if (option_value.empty ()) {
		_args.push_back (option_name);
		return;
	}

	std::string value_arg { option_name };
	value_arg.append ("=");
	value_arg.append (option_value);

	_args.push_back (value_arg);
}

void Process::append_program_argument (std::string const& option_name, string_list const& option_value, bool uses_comma_separated_list)
{
	if (option_value.empty ()) {
		append_program_argument (option_name);
		return;
	}

	if (!uses_comma_separated_list) {
		for (std::string const& value : option_value) {
			append_program_argument (option_name, value);
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

	_args.push_back (value_arg);
}
