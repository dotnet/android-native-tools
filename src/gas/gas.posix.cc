// SPDX-License-Identifier: MIT

#include <cstring>
#include <vector>

#include <libgen.h>

#include "gas.hh"
#include "platform.hh"

using namespace xamarin::android::gas;

void Gas::dump_command_line_args ([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{}

void Gas::platform_setup()
{}

void Gas::determine_program_dir (std::vector<platform::string> args)
{
	fs::path program_path { args[0] };

	_program_name = program_path.filename ();
	if (program_path.is_absolute ()) {
		_program_dir = program_path.parent_path ().make_preferred ();
	} else {
		_program_dir = fs::absolute (program_path).parent_path ().make_preferred ();
	}
}
