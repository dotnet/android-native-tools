// SPDX-License-Identifier: MIT
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <cerrno>
#include <memory>

#include "constants.hh"
#include "llvm_mc_runner.hh"
#include "process.hh"

using namespace xamarin::android::gas;

// Value is `true` if the option can be set multiple times
std::unordered_map<LlvmMcArgument, bool> LlvmMcRunner::known_options {
	{ LlvmMcArgument::Arch,          false },
	{ LlvmMcArgument::FileType,      false },
	{ LlvmMcArgument::IncludeDir,    true },
	{ LlvmMcArgument::Mcpu,          false },
	{ LlvmMcArgument::Output,        false },
	{ LlvmMcArgument::Mattr,         true },
	{ LlvmMcArgument::GenerateDebug, false},
};

int LlvmMcRunner::run (fs::path const& executable_path)
{
	auto process = std::make_unique<Process> (executable_path);

	auto opt = arguments.find (LlvmMcArgument::Arch);
	if (opt != arguments.end ()) {
		process->append_program_argument ("--arch", opt->second);
	}

	process->append_program_argument ("--triple", triple);
	process->append_program_argument ("--assemble");

	opt = arguments.find (LlvmMcArgument::GenerateDebug);
	if (opt != arguments.end ()) {
		process->append_program_argument ("-g");
	}

	opt = arguments.find (LlvmMcArgument::FileType);
	if (opt != arguments.end ()) {
		process->append_program_argument ("--filetype", opt->second);
	}

	opt = arguments.find (LlvmMcArgument::Mattr);
	if (opt != arguments.end ()) {
		process->append_program_argument ("--mattr", opt->second, true /* uses_comma_separated_list */);
	}

	opt = arguments.find (LlvmMcArgument::Output);
	if (opt != arguments.end ()) {
		process->append_program_argument ("-o", opt->second);
	}

	std::string input_file { "\"" + input_file_path.make_preferred ().string () + "\"" };
	process->append_program_argument (input_file_path.make_preferred ().string ());

	return process->run ();
}
