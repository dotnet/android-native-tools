// SPDX-License-Identifier: MIT
#include <windows.h>
#include <shlwapi.h>

#include <algorithm>
#include <cstring>
#include <string>
#include "gas.hh"

using namespace xamarin::android::gas;

void Gas::init_platform ()
{
	newline = "\r\n";

	auto make_exe = [](std::string& name)
	{
		name.append ("exe");
	};

	make_exe (arm64_program_name);
	make_exe (arm32_program_name);
	make_exe (x86_program_name);
	make_exe (x64_program_name);
}

void Gas::determine_program_name ([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: if the returned name is the generic one, look for the first argument passed to the program, if it's
	// `@name=NAME` then use NAME instead of looking for the process name. This is to allow for invoking `as` via
	// .cmd scripts, which saves space on disk (we don't need to copy the .exe around)
	TCHAR buffer[MAX_PATH + 1]{};
	constexpr DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

	if (GetModuleFileName (NULL, buffer, bufSize) == bufSize) {
		// TODO: throw
	}

	std::string name { PathFindFileName (buffer) };
	std::transform (
		name.begin (),
		name.end (),
		name.begin (),
		[](unsigned char c) { return std::tolower(c); }
	);

	_program_name = strdup (name.c_str ());
}
