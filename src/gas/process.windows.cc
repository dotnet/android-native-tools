// SPDX-License-Identifier: MIT
#include <windows.h>
#include <synchapi.h>
#include <tchar.h>
//#include <unistd.h>
#include <iostream>

#include "constants.hh"
#include "platform.hh"
#include "process.hh"

using namespace xamarin::android::gas;

static platform::string escape_argument (platform::string arg)
{
	bool needs_quote = false;

	for (platform::string::value_type const& c : arg) {
		if (isalpha (c) || isdigit (c) ||
		    c == '.' || c == '_' || c == '-' ||
		    c == '+' || c == '/') {
			continue;
		}

		needs_quote = true;
		break;
	}

	if (!needs_quote) {
		return arg;
	}

	platform::string result = L"\"";
	for (platform::string::const_reference c : arg) {
		if (c == L'\"' || c == L'\\') {
			result.append (L"\\");
		}

		result += c;
	}

	result.append (L"\"");
	return result;
}

int Process::run (bool print_command_line)
{
	if (print_command_line) {
		print_process_command_line ();
	}

	platform::string binary = executable_path.native ();
	platform::string args { escape_argument (binary) };
	for (platform::string const& a : _args) {
		if (a.empty ()) {
			continue;
		}
		args.append (L" ");
		args.append (escape_argument (a));
	}

	// int size =  MultiByteToWideChar (CP_UTF8, 0, args.c_str (), -1, NULL , 0);
	// wchar_t* wargs = new wchar_t [size];
	// MultiByteToWideChar (CP_UTF8, 0, args.c_str (), -1, wargs, size);

	// size =  MultiByteToWideChar (CP_UTF8, 0, binary.c_str (), -1, NULL , 0);
	// wchar_t* wbinary = new wchar_t [size];
	// MultiByteToWideChar (CP_UTF8, 0, binary.c_str (), -1, wbinary, size);

	PROCESS_INFORMATION pi {};
	STARTUPINFOW si {};
	si.cb = sizeof(si);

	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	BOOL success = CreateProcessW (
		const_cast<LPWSTR>(binary.c_str ()),
		const_cast<LPWSTR>(args.c_str ()),
		nullptr, // process security attributes
		nullptr, // primary thread security attributes
		TRUE, // inherit handles
		creation_flags,
		nullptr, // environment
		nullptr, // cwd, nullptr means it inheris from the parent process
		&si,
		&pi
	);

	// delete[] wargs;
	// delete[] wbinary;

	if (!success) {
		return Constants::wrapper_exec_failed_error_code;
	}

	// TODO: error handling below
	DWORD result = WaitForSingleObject (pi.hProcess, INFINITE);
	if (result == 0) {
		DWORD retcode = 0;
		if (GetExitCodeProcess (pi.hProcess, &retcode)) {
			return retcode;
		}

		return 128;
	}

	return 1;
}
