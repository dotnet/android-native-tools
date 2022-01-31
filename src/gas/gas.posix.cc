// SPDX-License-Identifier: MIT
#include <libgen.h>
#include <cstring>

#include "gas.hh"

using namespace xamarin::android::gas;

void Gas::determine_program_dir ([[maybe_unused]] int argc, char **argv)
{
	fs::path program_path { argv[0] };

	_program_name = program_path.filename ();
	if (program_path.is_absolute ()) {
		_program_dir = program_path.parent_path ().make_preferred ();
	} else {
		_program_dir = fs::absolute (program_path).parent_path ().make_preferred ();
	}
}
