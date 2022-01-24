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

	std::string args;
	for (std::string const& a : _args) {
		if (a.empty ()) {
			continue;
		}

		if (!args.empty ()) {
			args.append (" ");
		}
		args.append (escape_argument (a));
	}

	PROCESS_INFORMATION pi {};
	STARTUPINFO si         {};
	si.cb = sizeof(si);

	std::string binary = executable_path.string ();
	DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
	std::cout << " Calling CreateProcess" << Constants::newline;
	BOOL success = CreateProcess (
		binary.c_str (),
		const_cast<LPSTR>(args.c_str ()),
		nullptr, // process security attributes
		nullptr, // primary thread security attributes
		TRUE, // inherit handles
		creation_flags,
		nullptr, // environment
		nullptr, // cwd, nullptr means it inheris from the parent process
		&si,
		&pi
	);

	std::cout << " CreateProcess returned " << (success ? "true" : "false") << Constants::newline;
	if (!success) {
		return Constants::wrapper_exec_failed_error_code;
	}

	// TODO: error handling below
	std::cout << " Waiting for process " << std::hex << "0x" << pi.hProcess << " to exit" << Constants::newline;
	DWORD result = WaitForSingleObject (pi.hProcess, INFINITE);

	std::cout << " WaitForSingleObject returned " << result << Constants::newline;
	if (result == 0) {
		DWORD retcode = 0;
		if (GetExitCodeProcess (pi.hProcess, &retcode)) {
			std::cout << " process exited with code " << retcode << Constants::newline;
			return retcode;
		}

		std::cout << " GetExitCodeProcess failed, returning 1" << Constants::newline;
		return 1;
	}

	return 1;
}
