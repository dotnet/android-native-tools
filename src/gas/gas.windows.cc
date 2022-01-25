// SPDX-License-Identifier: MIT
#include <windows.h>
#include <shlwapi.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include "constants.hh"
#include "exceptions.hh"
#include "gas.hh"

using namespace xamarin::android::gas;

void Gas::determine_program_name (int argc, char **argv)
{
	TCHAR buffer[MAX_PATH + 1]{};
	constexpr DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

	if (GetModuleFileName (NULL, buffer, bufSize) == bufSize) {
		throw invalid_operation_error { "Unable to determine process name" };
	}

	auto lowercase_string = [](std::string& s) {
		std::transform (
			s.begin (),
			s.end (),
			s.begin (),
			[](unsigned char c) { return std::tolower(c); }
		);
	};

	std::string name { PathFindFileName (buffer) };
	lowercase_string (name);

	if (name.compare (generic_gas_name) == 0) {
		// A hack for Windows, since we can't (easily) use symlinks.
		const char *first_param = argc > 1 ? argv[1] : nullptr;
		if (first_param != nullptr && strlen (first_param) > sizeof(Constants::name_hack_param) && strstr (first_param, Constants::name_hack_param) == first_param) {
			name = first_param + (sizeof(Constants::name_hack_param) - 1);
			lowercase_string (name);
			if (!name.ends_with (".exe")) {
				name += ".exe";
			}
		}
	}

	_program_name = name;
	PathRemoveFileSpec (buffer);
	_program_dir = buffer;
}
