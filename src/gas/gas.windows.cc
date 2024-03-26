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
