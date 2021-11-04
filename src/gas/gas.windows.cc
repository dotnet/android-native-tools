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
