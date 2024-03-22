// SPDX-License-Identifier: MIT
#include <sys/types.h>

#include <cstring>
#include <cerrno>
#include <memory>

#include "constants.hh"
#include "llvm_mc_runner.hh"
#include "platform.hh"
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
		process->append_program_argument (PSTR("--arch"), opt->second);
	}

	process->append_program_argument (PSTR("--triple"), triple);
	process->append_program_argument (PSTR("--assemble"));

	opt = arguments.find (LlvmMcArgument::GenerateDebug);
	if (opt != arguments.end ()) {
		process->append_program_argument (PSTR("-g"));
	}

	opt = arguments.find (LlvmMcArgument::FileType);
	if (opt != arguments.end ()) {
		process->append_program_argument (PSTR("--filetype"), opt->second);
	}

	opt = arguments.find (LlvmMcArgument::Mattr);
	if (opt != arguments.end ()) {
		process->append_program_argument (PSTR("--mattr"), opt->second, true /* uses_comma_separated_list */);
	}

	opt = arguments.find (LlvmMcArgument::Output);
	if (opt != arguments.end ()) {
		process->append_program_argument (PSTR("-o"), opt->second);
	}

	platform::string input_file { PSTR("\"") + input_file_path.make_preferred ().native () + PSTR("\"") };
	process->append_program_argument (input_file_path.make_preferred ().native ());

	return process->run ();
}
