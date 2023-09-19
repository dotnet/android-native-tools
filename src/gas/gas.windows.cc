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

void Gas::get_command_line (int &argc, char **&argv)
{
	LPWSTR *argvw = CommandLineToArgvW (GetCommandLineW (), &argc);
	argv = new char*[argc + 1];

	for (int i = 0; i < argc; i++) {
		int size = WideCharToMultiByte (CP_UTF8, 0, argvw [i], -1, NULL, 0, NULL, NULL);
		argv [i] = new char [size];
		WideCharToMultiByte (CP_UTF8, 0, argvw [i], -1, argv [i], size, NULL, NULL);
	}

	argv [argc] = NULL;
}

void Gas::determine_program_dir ([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	TCHAR buffer[MAX_PATH + 1]{};
	constexpr DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

	if (GetModuleFileName (NULL, buffer, bufSize) == bufSize) {
		throw invalid_operation_error { "Unable to determine process name" };
	}

	PathRemoveFileSpec (buffer);
	_program_dir = buffer;
}
