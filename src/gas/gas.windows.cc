// SPDX-License-Identifier: MIT
#include <windows.h>
#include <shlwapi.h>

#include <cstdio>
#include <clocale>
#include <cstring>
#include <iostream>
#include <vector>
#include <io.h>
#include <fcntl.h>

#include "exceptions.hh"
#include "gas.hh"
#include "platform.hh"

using namespace xamarin::android::gas;

namespace {
	void log_cp_info(std::wstring label, UINT cp)
	{
		std::wcout << L"  " << label << L" code page:" << std::endl;
		std::wcout << L"    ID: " << cp << std::endl;

		CPINFOEX cpinfo;
		BOOL result = GetCPInfoEx(cp, 0, &cpinfo);
		if (!result) {
			std::wcout << L"    failed to obtain more information about the code page" << std::endl;
			return;
		}

		std::wcout
			<< L"    Maximum character size: " << cpinfo.MaxCharSize << std::endl
			<< L"    Localized name: " << cpinfo.CodePageName << std::endl;
	}

	void log_cli_arg(int index, const wchar_t* arg)
	{
		std::wcout << L"  [" << index << L"] " << std::endl;
		std::wcout << std::endl;
		std::wstring ws(arg);
		std::wcout << L"    As C string (direct): " << ws << std::endl;
		std::wcout << L"    As hex bytes: " << std::hex << std::setw(4) << std::setfill(L'0');

		const wchar_t* p = arg;
		while (p != nullptr && *p != 0) {
			auto ch = static_cast<uint16_t>(*p);
			std::wcout << ch << " ";
			p++;
		}
		std::wcout << std::endl;
	}
}

void Gas::dump_command_line_args (int argc, wchar_t **argv)
{
	std::wcout << L"Active code pages information" << std::endl;
	log_cp_info(L"OS", GetACP());
	std::wcout << std::endl;
	log_cp_info(L"OEM", GetOEMCP());
	std::wcout << std::endl;

	std::wcout << L"Command line arguments (" << argc << "):" << std::endl;
	for (int i = 0; i < argc; i++) {
		log_cli_arg(i, argv[i]);
	}
	std::wcout << L"================================" << std::endl << std::endl;
}

void Gas::platform_setup()
{
	// Windows needs that magic to make stdout work with wchar_t and friends
	constexpr char cp_utf16le[] = ".1200"; // UTF-16 little-endian locale.
	setlocale(LC_ALL, cp_utf16le);
	_setmode(_fileno(stdout), _O_WTEXT);
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
