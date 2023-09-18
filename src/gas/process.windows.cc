// SPDX-License-Identifier: MIT
#include <windows.h>
#include <synchapi.h>
#include <tchar.h>
#include <unistd.h>
#include <iostream>

#include "constants.hh"
#include "process.hh"

using namespace xamarin::android::gas;

static std::string escape_argument (std::string arg)
{
	bool needs_quote = false;

	for (std::string::value_type const& c : arg) {
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

	std::string result = "\"";
	for (std::string::const_reference c : arg) {
		if (c == '\"' || c == '\\') {
			result.append ("\\");
		}

		result += c;
	}

	result.append ("\"");
	return result;
}

int Process::run (bool print_command_line)
{
	if (print_command_line) {
		print_process_command_line ();
	}

	std::string binary = executable_path.string ();
	std::string args { escape_argument (binary) };
	for (std::string const& a : _args) {
		if (a.empty ()) {
			continue;
		}
		args.append (" ");
		args.append (escape_argument (a));
	}

	int size =  MultiByteToWideChar (CP_UTF8, 0, args.c_str (), -1, NULL , 0);
	wchar_t* wargs = new wchar_t [size];
	MultiByteToWideChar (CP_UTF8, 0, args.c_str (), -1, wargs, size);

	size =  MultiByteToWideChar (CP_UTF8, 0, binary.c_str (), -1, NULL , 0);
	wchar_t* wbinary = new wchar_t [size];
	MultiByteToWideChar (CP_UTF8, 0, binary.c_str (), -1, wbinary, size);

	PROCESS_INFORMATION pi {};
	STARTUPINFOW si {};
	si.cb = sizeof(si);

	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	BOOL success = CreateProcessW (
		wbinary,
		wargs,
		nullptr, // process security attributes
		nullptr, // primary thread security attributes
		TRUE, // inherit handles
		creation_flags,
		nullptr, // environment
		nullptr, // cwd, nullptr means it inheris from the parent process
		&si,
		&pi
	);

	delete wargs;
	delete wbinary;

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
