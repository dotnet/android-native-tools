// SPDX-License-Identifier: MIT
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>

#include "constants.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

int LlvmMcRunner::run_process (fs::path const& executable_path, std::vector<std::string::const_pointer> const& exec_args)
{
	pid_t llvm_mc_pid = fork ();
	if (llvm_mc_pid == -1) {
		std::cerr << "Fork failed. " << std::strerror (errno) << Constants::newline;
		return Constants::wrapper_fork_failed_error_code;
	}

	if (llvm_mc_pid == 0) {
		if (execv (executable_path.c_str (), const_cast<char* const*>(exec_args.data ())) == -1) {
			std::cerr << "Failed to run " << Constants::llvm_mc_name << ". " << std::strerror (errno) << Constants::newline;
		}
		_exit (Constants::wrapper_exec_failed_error_code);
	}

	int wstatus = 0;
	do {
		pid_t result = waitpid (llvm_mc_pid,  &wstatus, WUNTRACED);
		if (result == -1) {
			std::cerr << "Failed to wait for " << Constants::llvm_mc_name << " to terminate. " << std::strerror (errno) << Constants::newline;
			return Constants::wrapper_wait_failed_error_code;
		}

		if (WIFSIGNALED (wstatus)) {
			std::cerr << Constants::llvm_mc_name << " was killed by signal " << WTERMSIG (wstatus) << Constants::newline;
			return Constants::wrapper_llvm_mc_killed_error_code;
		} else if (WIFSTOPPED (wstatus)) {
			std::cerr << Constants::llvm_mc_name << " was stopped by signal " << WSTOPSIG (wstatus) << Constants::newline;
			kill (llvm_mc_pid, SIGKILL); // Let's not risk hanging indifinitely...
			return Constants::wrapper_llvm_mc_stopped_error_code;
		}
	} while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

	if (WEXITSTATUS (wstatus) != 0) {
		std::cerr << Constants::llvm_mc_name << " exited with status " << WEXITSTATUS (wstatus) << Constants::newline;
	}
	return WEXITSTATUS (wstatus);
}
