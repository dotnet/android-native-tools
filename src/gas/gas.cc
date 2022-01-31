// SPDX-License-Identifier: MIT
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <filesystem>

#include "constants.hh"
#include "gas.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

int Gas::usage (bool is_error, std::string const message)
{
	if (!message.empty ()) {
		std::cerr << message << Constants::newline << Constants::newline;
	}

	std::cerr << "`" << program_name () << "` takes a subset of arguments accepted by the GNU Assembler (gas) program." << Constants::newline
	          << "Accepted options are limited to the ones used by Xamarin.Android and Mono/dotnet AOT, more can be added as-needed." << Constants::newline
	          << "Some options are accepted but ignored, either because they are ignored by GAS as well or because they are used by" << Constants::newline
	          << "Xamarin.Android or Mono/dotnet AOT, but not needed by `llvm-mc`.  All the unsupported options will cause the wrapper" << Constants::newline
	          << "to fail with an error message." << Constants::newline
	          << "Since `llvm-mc` does not support compiling multiple source files at the same time, this GAS behavior is emulated" << Constants::newline
	          << "by running `llvm-mc` once per each input file and using the `ld` linker in the end to merge all the discrete output" << Constants::newline
	          << "files into the single file indicated by the `-o` option." << Constants::newline << Constants::newline
	          << "Command line options are compatibile with GAS version " << BINUTILS_VERSION << Constants::newline << Constants::newline
	          << "Currently supported options are:" << Constants::newline << Constants::newline
	          << "All targets" << Constants::newline
	          << "   -o FILE            path to the output object file" << Constants::newline
	          << "  --warn              don't suppress warning messages" << Constants::newline
	          << "   -g | --gen-debug   generate debug information in the output object file" << Constants::newline << Constants::newline
	          << "x86/x86_64 targets" << Constants::newline
	          << "  --32                output a 32-bit object [ignored, `llvm-mc` is always invoked for the right target]" << Constants::newline
	          << "  --64                output a 64-bit object [ignored, as above]" << Constants::newline << Constants::newline
	          << "armeabi/arm32 targets" << Constants::newline
	          << "  -mfpu=FPU           select floating-point architecture for the target" << Constants::newline << Constants::newline
	          << "Ignored by GAS and this wrapper" << Constants::newline
	          << "  --divide" << Constants::newline
	          << "   -k" << Constants::newline
	          << "  --nocpp" << Constants::newline
	          << "   -Qn" << Constants::newline
	          << "   -Qy" << Constants::newline
	          << "   -s" << Constants::newline
	          << "   -w" << Constants::newline
	          << "   -X" << Constants::newline << Constants::newline
	          << "Wrapper options, not passed to `llvm-mc`" << Constants::newline
	          << "   -h | --help        show this help screen" << Constants::newline
	          << "   -V                 show version" << Constants::newline
	          << "  --version           show version and exit " << Constants::newline
	          << Constants::newline;

	return is_error ? 1 : 0;
}

int Gas::run (int argc, char **argv)
{
	determine_program_dir (argc, argv);

	auto lowercase_string = [](std::string& s) {
		std::transform (
			s.begin (),
			s.end (),
			s.begin (),
			[](unsigned char c) { return std::tolower(c); }
		);
	};

	std::string arch_name { generic_gas_name };
	const char *first_param = argc > 1 ? argv[1] : nullptr;
	if (first_param != nullptr && strlen (first_param) > sizeof(Constants::arch_hack_param) && strstr (first_param, Constants::arch_hack_param) == first_param) {
		arch_name = first_param + (sizeof(Constants::arch_hack_param) - 1);
		lowercase_string (arch_name);
	}
	_program_name = arch_name;

	std::unique_ptr<LlvmMcRunner> mc_runner;
	std::string ld_name;
	if (arch_name.compare (arm64_gas_name.data ()) == 0) {
		_target_arch = TargetArchitecture::ARM64;
		mc_runner = std::make_unique<LlvmMcRunnerARM64> ();
		ld_name = arm64_ld_name.data ();
	} else if (arch_name.compare (arm32_gas_name.data ()) == 0) {
		_target_arch = TargetArchitecture::ARM32;
		mc_runner = std::make_unique<LlvmMcRunnerARM32> ();
		ld_name = arm32_ld_name.data ();
	} else if (arch_name.compare (x86_gas_name.data ()) == 0) {
		_target_arch = TargetArchitecture::X86;
		mc_runner = std::make_unique<LlvmMcRunnerX86> ();
		ld_name = x86_ld_name.data ();
	} else if (arch_name.compare (x64_gas_name.data ()) == 0) {
		_target_arch = TargetArchitecture::X64;
		mc_runner = std::make_unique<LlvmMcRunnerX64> ();
		ld_name = x64_ld_name.data ();
	} else if (arch_name.compare (generic_gas_name) == 0) {
		std::string message { "Program invoked via its generic name (" };
		message
			.append (generic_gas_name)
			.append ("), please use one of the ABI-prefixed names:")
			.append (Constants::newline)
			.append ("  ").append (arm64_gas_name.data ()).append (Constants::newline)
			.append ("  ").append (arm32_gas_name.data ()).append (Constants::newline)
			.append ("  ").append (x86_gas_name.data ()).append (Constants::newline)
			.append ("  ").append (x64_gas_name.data ()).append (Constants::newline);
		return usage (true /* is_error */, message);
	} else {
		std::string message { "Unknown program name '" };
		message.append (arch_name).append ("'").append (Constants::newline);
		return usage (true /* is_error */, message);
	}

	auto&& [terminate, is_error] = parse_arguments (argc, argv, mc_runner);
	if (terminate || is_error) {
		return is_error ? Constants::wrapper_general_error_code : 0;
	}

	fs::path llvm_mc = program_dir () / Constants::llvm_mc_name;

	bool multiple_input_files = false;
	bool derive_output_file_name = false;
	switch (input_files.size ()) {
		case 0:
			return usage (true, "missing input files on command line");

		case 1:
			// We should always have a value here since `a.out` is the default, but... :)
			if (!_gas_output_file.empty ()) {
				mc_runner->set_output_file_path (_gas_output_file);
			} else {
				derive_output_file_name = true;
			}
			break;

		default:
			multiple_input_files = true;
			derive_output_file_name = true;
			break;
	}

	for (fs::path const& input : input_files) {
		mc_runner->set_input_file_path (input, derive_output_file_name);
		int ret = mc_runner->run (llvm_mc);
		if (ret != 0) {
			return ret;
		}
	}

	if (multiple_input_files) {
		std::vector<fs::path> output_files;
		for (fs::path const& input : input_files) {
			output_files.push_back (mc_runner->make_output_file_path (input));
		}

		fs::path ld_path { program_dir () };
		ld_path /= ld_name;
		auto ld = std::make_unique<Process> (ld_path);
		ld->append_program_argument ("-o");
		ld->append_program_argument (_gas_output_file.empty () ? Constants::default_output_name : _gas_output_file.string ());
		ld->append_program_argument ("--relocatable");

		std::cout << "Have multiple output files:" << Constants::newline;
		for (fs::path const& output : output_files) {
			std::cout << "  " << output << Constants::newline;
			ld->append_program_argument (output.string ());
		}

		return ld->run ();
	}

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
			{
				// Ignore the arch hack parameter
				const char *ret = strstr (optarg, Constants::arch_hack_param);
				if (ret == nullptr || ret != optarg) {
					input_files.emplace_back (optarg);
				}
			}
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

			case OPTION_G:
				mc_runner->generate_debug_info ();
				break;
		}
	}

	if (show_help) {
		usage (false /* is_error */);
		return {true, false};
	} else if (show_version) {
		std::cout << program_name () << " v" << XA_UTILS_VERSION << ", " << PROGRAM_DESCRIPTION << Constants::newline
		          << "\tGAS version compatibility: " << BINUTILS_VERSION << Constants::newline
		          << "\tllvm-mc version compatibility: " << LLVM_VERSION << Constants::newline << Constants::newline;
		return {true, false};
	}

	if (terminate) {
		return {terminate, is_error};
	}

	if (_gas_output_file.empty ()) {
		_gas_output_file = Constants::default_output_name;
	}

 	return {terminate, is_error};
}
