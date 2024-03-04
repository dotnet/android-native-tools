// SPDX-License-Identifier: MIT
#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "shared-constants.hh"
#include "process.hh"
#include "scope-guard.hh"

using namespace xamarin::android::binutils;

int Process::run (bool print_command_line)
{
	if (print_command_line) {
		print_process_command_line ();
	}

	std::vector<std::string::const_pointer> exec_args = make_exec_args ();

	ScopeGuard fg {
		[&]() -> void {
			// We made a copy in `make_exec_args`, free it up here
			free (const_cast<char*>(exec_args[0]));
		}
	};

	// `execv(2)` needs the array to be null-terminated
	exec_args.push_back (nullptr);

	pid_t llvm_mc_pid = fork ();
	if (llvm_mc_pid == -1) {
		std::cerr << "Fork failed. " << std::strerror (errno) << SharedConstants::newline;
		return SharedConstants::wrapper_fork_failed_error_code;
	}

	if (llvm_mc_pid == 0) {
		if (execv (executable_path.c_str (), const_cast<char* const*>(exec_args.data ())) == -1) {
			std::cerr << "Failed to run " << executable_path << ". " << std::strerror (errno) << SharedConstants::newline;
		}
		_exit (SharedConstants::wrapper_exec_failed_error_code);
	}

	int wstatus = 0;
	do {
		pid_t result = waitpid (llvm_mc_pid,  &wstatus, WUNTRACED);

		if (result == -1) {
			std::cerr << "Failed to wait for " << executable_path << " to terminate. " << std::strerror (errno) << SharedConstants::newline;
			return SharedConstants::wrapper_wait_failed_error_code;
		}

		if (WIFSIGNALED (wstatus)) {
			std::cerr << executable_path << " was killed by signal " << WTERMSIG (wstatus) << SharedConstants::newline;
			return SharedConstants::wrapper_process_killed_error_code;
		} else if (WIFSTOPPED (wstatus)) {
			std::cerr << executable_path << " was stopped by signal " << WSTOPSIG (wstatus) << SharedConstants::newline;
			kill (llvm_mc_pid, SIGKILL); // Let's not risk hanging indifinitely...
			return SharedConstants::wrapper_process_stopped_error_code;
		}
	} while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

	if (WEXITSTATUS (wstatus) != 0) {
		std::cerr << executable_path << " exited with status " << WEXITSTATUS (wstatus) << SharedConstants::newline;
	}
	return WEXITSTATUS (wstatus);
}
