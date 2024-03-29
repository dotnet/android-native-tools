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

	platform::string result = PSTR("\"");
	for (platform::string::const_reference c : arg) {
		if (c == PCHAR('\"') || c == PCHAR('\\')) {
			result.append (PCHAR("\\"));
		}

		result += c;
	}

	result.append (PSTR("\""));
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
		args.append (PCHAR(" "));
		args.append (escape_argument (a));
	}

	PROCESS_INFORMATION pi {};
	STARTUPINFOW si {};
	si.cb = sizeof(si);

	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	wchar_t* wargs = _wcsdup(args.c_str());
	BOOL success = CreateProcessW (
		binary.c_str (),
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
	free (wargs);

	if (!success) {
		return Constants::wrapper_exec_failed_error_code;
	}

	// TODO: error handling below
	int ret = 0;
	DWORD result = WaitForSingleObject (pi.hProcess, INFINITE);
	if (result == 0) {
		DWORD retcode = 0;
		if (GetExitCodeProcess (pi.hProcess, &retcode)) {
			ret = retcode;
		} else {
			ret = 128;
		}
	} else {
		ret = 1;
	}

	CloseHandle (pi.hProcess);
	CloseHandle (pi.hThread);

	return ret;
}
