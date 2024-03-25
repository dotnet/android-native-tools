// SPDX-License-Identifier: MIT
#include <windows.h>
#include <shlwapi.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "exceptions.hh"
#include "gas.hh"
#include "platform.hh"

using namespace xamarin::android::gas;

std::vector<platform::string> Gas::get_command_line (int &argc, char **&argv)
{
	for (int i = 0; i < argc; i++) {
		std::printf ("[A][%d] %s\n", i, argv[i]);
	}

	LPWSTR *szArglist;
	int nArgs;
	int i;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL == szArglist ) {
		wprintf(L"CommandLineToArgvW failed\n");
		return {};
	} else for( i=0; i<nArgs; i++) printf("[W] %d: %ws\n", i, szArglist[i]);

	int n_args;
	LPWSTR *argv_wide = CommandLineToArgvW (GetCommandLineW (), &n_args);
	std::vector<platform::string> ret;

	for (int i = 0; i < n_args; i++) {
		std::printf ("[W][%d] %ws\n", i, argv_wide[i]);
		ret.emplace_back (argv_wide[i]);
	}

	return ret;
}

void Gas::determine_program_dir (std::vector<platform::string> args)
{
	TCHAR buffer[MAX_PATH + 1]{};
	constexpr DWORD bufSize = sizeof(buffer) / sizeof(*buffer);

	if (GetModuleFileName (NULL, buffer, bufSize) == bufSize) {
		throw invalid_operation_error { "Unable to determine process name" };
	}

	PathRemoveFileSpec (buffer);
	_program_dir = buffer;
}
