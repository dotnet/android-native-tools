// SPDX-License-Identifier: MIT
#include <cstring>
#include <iostream>

#include "platform.hh"
#include "process.hh"

using namespace xamarin::android::gas;

void Process::print_process_command_line ()
{
	STDOUT << "Running: " << executable_path;
	for (platform::string const& arg : _args) {
		STDOUT << " " << arg;
	}
	STDOUT << Constants::newline;
}

void Process::append_program_argument (platform::string const& option_name, platform::string const& option_value)
{
	if (option_value.empty ()) {
		_args.push_back (option_name);
		return;
	}

	platform::string value_arg { option_name };
	value_arg.append (PSTR("="));
	value_arg.append (option_value);

	_args.push_back (value_arg);
}

void Process::append_program_argument (platform::string const& option_name, string_list const& option_value, bool uses_comma_separated_list)
{
	if (option_value.empty ()) {
		append_program_argument (option_name);
		return;
	}

	if (!uses_comma_separated_list) {
		for (platform::string const& value : option_value) {
			append_program_argument (option_name, value);
		}
		return;
	}

	platform::string value_arg { option_name };
	value_arg.append (PSTR("="));

	platform::string value_list;
	for (platform::string const& value : option_value) {
		if (!value_list.empty ()) {
			value_list.append (PSTR(","));
		}
		value_list.append (value);
	}
	value_arg.append (value_list);

	_args.push_back (value_arg);
}
