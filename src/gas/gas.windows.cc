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
