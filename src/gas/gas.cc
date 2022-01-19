// SPDX-License-Identifier: MIT
#include <unistd.h>
#include <iostream>

#include "gas.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

Gas::Gas ()
{
	arm64_program_name = make_program_name (arm64_arch_prefix);
	arm32_program_name = make_program_name (arm32_arch_prefix);
	x86_program_name = make_program_name (x86_arch_prefix);
	x64_program_name = make_program_name (x64_arch_prefix);

	init_platform ();
}

int Gas::usage (bool is_error, std::string const message)
{
	if (!message.empty ()) {
		std::cerr << message << newline << newline;
	}

	std::cerr << "`" << program_name () << "` takes the same arguments as the GNU Assembler (gas) program." << newline
	          << "Please read the `as(1)` manual page or visit https://sourceware.org/binutils/docs-" << BINUTILS_VERSION << "/as/Invoking.html#Invoking" << newline
	          << "Command line options are compatibile with GAS version " << BINUTILS_VERSION << newline
	          << newline;

	return is_error ? 1 : 0;
}

int Gas::run (int argc, char **argv)
{
	determine_program_name (argc, argv);
	std::cout << "Program name: " << program_name () << newline;
	std::cout << "Program dir: " << program_dir () << newline << newline;

	std::unique_ptr<LlvmMcRunner> mc_runner;
	if (arm64_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::ARM64;
		mc_runner = std::make_unique<LlvmMcRunnerARM64> ();
	} else if (arm32_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::ARM32;
		mc_runner = std::make_unique<LlvmMcRunnerARM32> ();
	} else if (x86_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::X86;
		mc_runner = std::make_unique<LlvmMcRunnerX86> ();
	} else if (x64_program_name.compare (program_name ()) == 0) {
		_target_arch = TargetArchitecture::X64;
		mc_runner = std::make_unique<LlvmMcRunnerX64> ();
	} else if (generic_program_name.compare (program_name ()) == 0) {
		std::string message { "Program invoked via its generic name (" };
		message
			.append (generic_program_name)
			.append ("), please use one of the ABI-prefixed names:")
			.append (newline)
			.append ("  ").append (arm64_program_name).append (newline)
			.append ("  ").append (arm32_program_name).append (newline)
			.append ("  ").append (x86_program_name).append (newline)
			.append ("  ").append (x64_program_name).append (newline);
		return usage (true /* is_error */, message);
	} else {
		std::string message { "Unknown program name '" };
		message.append (program_name ()).append ("'").append (newline);
		return usage (true /* is_error */, message);
	}

	auto&& [terminate, is_error] = parse_arguments (argc, argv, mc_runner);
	if (terminate || is_error) {
		return is_error ? 1 : 0;
	}

	fs::path llvm_mc = program_dir () / llvm_mc_name;

	switch (input_files.size ()) {
		case 0:
			return usage (true, "missing input files on command line");

		case 1:
			if (!_gas_output_file.empty ()) {
				mc_runner->set_output_file_path (_gas_output_file);
			}
			break;
	}

	for (fs::path const& input : input_files) {
		mc_runner->set_input_file_path (input, _gas_output_file.empty ());
		int ret = mc_runner->run (llvm_mc);
		if (ret < 0) {
			return ret;
		}
	}

	// TODO: run `ld --relocatable` at the end to produce the desired output file by merging the multiple .o files
	// produced by llvm-mc invocations for each input file, if there are multiple input files.

	return 0;
}

std::vector<option> Gas::common_options {
	// Arguments ignored by GAS, we shall ignore them silently too
	{ "divide",    no_argument,       nullptr, OPTION_IGNORE },
	{ "k",         no_argument,       nullptr, OPTION_IGNORE },
	{ "nocpp",     no_argument,       nullptr, OPTION_IGNORE },
	{ "Qn",        no_argument,       nullptr, OPTION_IGNORE },
	{ "Qy",        no_argument,       nullptr, OPTION_IGNORE },
	{ "s",         no_argument,       nullptr, OPTION_IGNORE },
	{ "w",         no_argument,       nullptr, OPTION_IGNORE },
	{ "X",         no_argument,       nullptr, OPTION_IGNORE },

	// Global GAS arguments we support
	{ "o",         required_argument, nullptr, OPTION_O },
	{ "warn",      no_argument,       nullptr, OPTION_WARN },
	{ "g",         no_argument,       nullptr, OPTION_G },
	{ "gen-debug", no_argument,       nullptr, OPTION_G },

	// Arguments handled by us, not passed to llvm-mc
	{ "h",         no_argument,       nullptr, OPTION_HELP },
	{ "help",      no_argument,       nullptr, OPTION_HELP },
	{ "V",         no_argument,       nullptr, OPTION_VERSION },
	{ "version",   no_argument,       nullptr, OPTION_VERSION_EXIT },
};

std::vector<option> Gas::x86_options {
	{ "32", no_argument, nullptr, OPTION_IGNORE }, // llvm-mc doesn't need this
	{ "64", no_argument, nullptr, OPTION_IGNORE }, // llvm-mc doesn't need this
};

std::vector<option> Gas::arm32_options {
	{ "mfpu", required_argument, nullptr, OPTION_MFPU },
};

Gas::ParseArgsResult Gas::parse_arguments (int argc, char **argv, std::unique_ptr<LlvmMcRunner>& mc_runner)
{
	std::vector<option> long_options { common_options };

	switch (target_arch ()) {
		case TargetArchitecture::ARM32:
			long_options.insert (long_options.end (), arm32_options.begin (), arm32_options.end ());
			break;

		case TargetArchitecture::X86:
		case TargetArchitecture::X64:
			long_options.insert (long_options.end (), x86_options.begin (), x86_options.end ());
			break;

		default:
			break;
	}
	long_options.push_back ({ nullptr, 0, nullptr, 0 });

	constexpr char PROGRAM_DESCRIPTION[] = "Xamarin.Android GAS adapter for llvm-mc";

	bool terminate = false, is_error = false;
	bool show_version = false, show_help = false;

	while (true) {
		int opt_index = 0;
		int c = getopt_long_only (argc, argv, "-", long_options.data (), &opt_index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case '?':
				terminate = true;
				is_error = true;
				break;

			case 1: // non-option argument
				input_files.emplace_back (optarg);
				break;

			case OPTION_VERSION:
				show_version = true;
				break;

			case OPTION_VERSION_EXIT:
				show_version = true;
				terminate = true;
				break;

			case OPTION_HELP:
				show_help = true;
				terminate = true;
				break;

			case OPTION_O:
				_gas_output_file = optarg;
				break;

			case OPTION_MFPU:
				mc_runner->map_option ("mfpu", optarg);
				break;

				// TODO: -o should cause an error if multiple input files are specified, llvm-mc doesn't support
				// compiling more than one file at a time. Alternatively, we could run `ld --relocatable` at the end to
				// produce the desired output file by merging the multiple .o files produced by llvm-mc invocations for
				// each input file.

			case OPTION_G:
				mc_runner->generate_debug_info ();
				break;
		}
	}

	if (show_help) {
		usage (false /* is_error */);
		return {true, false};
	} else if (show_version) {
		std::cout << program_name () << " vX.Y.Z, " << PROGRAM_DESCRIPTION << newline
		          << "\tGAS version compatibility: " << BINUTILS_VERSION << newline
		          << "\tllvm-mc version compatibility: " << LLVM_VERSION << newline << newline;
		return {true, false};
	}

	if (terminate) {
		return {terminate, is_error};
	}

 	return {terminate, is_error};
}
