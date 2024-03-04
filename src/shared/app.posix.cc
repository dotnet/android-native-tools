// SPDX-License-Identifier: MIT
#include <filesystem>

#include "app.hh"

using namespace xamarin::android::binutils;
namespace fs = std::filesystem;

void App::get_command_line ([[maybe_unused]] int &argc, [[maybe_unused]] char **&argv)
{
	// no-op
}

void App::determine_program_dir ([[maybe_unused]] int argc, char **argv)
{
	fs::path program_path { argv[0] };

	_program_name = program_path.filename ();
	if (program_path.is_absolute ()) {
		_program_dir = program_path.parent_path ().make_preferred ();
	} else {
		_program_dir = fs::absolute (program_path).parent_path ().make_preferred ();
	}
}
