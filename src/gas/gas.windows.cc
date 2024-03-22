// SPDX-License-Identifier: MIT
#include <windows.h>
#include <shlwapi.h>

#include <cstring>
#include <iostream>

#include "exceptions.hh"
#include "gas.hh"

using namespace xamarin::android::gas;

void Gas::get_command_line (int &argc, char **&argv)
{
	LPWSTR *argvw = CommandLineToArgvW (GetCommandLineW (), &argc);
	argv = new char*[argc + 1];
	memset (argv, 0, argc * sizeof(char*));
	for (int i = 0; i < argc; i++) {
		std::cout << "Argument " << i << std::endl;
		int size = WideCharToMultiByte (CP_UTF8, 0, argvw [i], -1, NULL, 0, NULL, NULL);
		std::cout << "  size: " << size << std::endl;
		if (size <= 0) {
			// An error, but we ignore it and try to get the other arguments converted.
			argv[i] = _strdup ("");
			continue;
		}

		argv [i] = new char [size];
		memset (argv [i], 0, size * sizeof(char));
		std::cout << "  copying" << std::endl;
		WideCharToMultiByte (CP_UTF8, 0, argvw [i], -1, argv [i], size, NULL, NULL);
		std::cout << "  copied" << std::endl;
	}

	std::cout << "Processed arguments: " << std::endl;
	for (int i = 0; i < argc; i++) {
		std::cout << "  [" << i << "] " << argv[i] << std::endl;
	}

	argv [argc] = NULL;
}

void Gas::determine_program_dir ([[maybe_unused]] int argc, [[maybe_unused]] CommandLine::TArgType *argv)
{
	TCHAR buffer[MAX_PATH + 1]{};
	constexpr DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

	if (GetModuleFileName (NULL, buffer, bufSize) == bufSize) {
		throw invalid_operation_error { "Unable to determine process name" };
	}

	PathRemoveFileSpec (buffer);
	_program_dir = buffer;
}
