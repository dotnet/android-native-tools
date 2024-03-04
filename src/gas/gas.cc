// SPDX-License-Identifier: MIT
#include <cstring>
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include "constants.hh"
#include "gas.hh"
#include "../shared/process.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::binutils;
using namespace xamarin::android::gas;

cxxopts::Options Gas::create_options ()
{
	cxxopts::Options options (program_name(), PROGRAM_DESCRIPTION.data ());
	options
		.positional_help ("[asmfile...]")
		.set_width (80);

	options.add_options ("Supported GAS arguments")
		("o",           "name the object-file output `arg` (default a.out)", cxxopts::value<std::string>())
		("warn",        "don't suppress warnings", cxxopts::value<bool>())
		("g,gen-debug", "generate debugging information", cxxopts::value<bool>())
		("asmfile",     "assembler source file(s)", cxxopts::value<std::vector<std::string>>());

	options.add_options ("Wrapper options, not passed to `llvm-mc`")
		("h,help", "show this help screen", cxxopts::value<bool>())
		("V", "show version", cxxopts::value<bool>())
		("version", "show version and exit", cxxopts::value<bool>());

	options.add_options ("Ignored by GAS and this wrapper")
		("divide", "ignored", cxxopts::value<bool>())
		("k", "ignored", cxxopts::value<bool>())
		("nocpp", "ignored", cxxopts::value<bool>())
		("Qn", "ignored", cxxopts::value<bool>())
		("Qy", "ignored", cxxopts::value<bool>())
		("s", "ignored", cxxopts::value<bool>())
		("w", "ignored", cxxopts::value<bool>())
		("X", "ignored", cxxopts::value<bool>());

	switch (target_arch ()) {
		case TargetArchitecture::ARM32:
			options.add_options ("Arm options")
				("mfpu", "assemble for FPU architecture <fpu name>", cxxopts::value<std::string>());
			break;

		case TargetArchitecture::X86:
		case TargetArchitecture::X64:
			options.add_options ("x86/x64 options")
				("32", "generate 32bit object", cxxopts::value<bool>())
				("64", "generate 64bit object", cxxopts::value<bool>());
			break;

		default:
			break;
	}

	return options;
}

void Gas::usage_print_extra ([[maybe_unused]] bool is_error)
{
	std::cerr << "`" << program_name () << "` takes a subset of arguments accepted by the GNU Assembler (gas) program." << SharedConstants::newline
	          << "Accepted options are limited to the ones used by Xamarin.Android and Mono/dotnet AOT, more can be added as-needed." << SharedConstants::newline
	          << "Some options are accepted but ignored, either because they are ignored by GAS as well or because they are used by" << SharedConstants::newline
	          << "Xamarin.Android or Mono/dotnet AOT, but not needed by `llvm-mc`.  All the unsupported options will cause the wrapper" << SharedConstants::newline
	          << "to fail with an error message." << SharedConstants::newline
	          << "Since `llvm-mc` does not support compiling multiple source files at the same time, this GAS behavior is emulated" << SharedConstants::newline
	          << "by running `llvm-mc` once per each input file and using the `ld` linker in the end to merge all the discrete output" << SharedConstants::newline
	          << "files into the single file indicated by the `-o` option." << SharedConstants::newline << SharedConstants::newline
	          << "Command line options are compatibile with GAS version " << BINUTILS_VERSION << SharedConstants::newline << SharedConstants::newline;
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
	set_program_name (arch_name);

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
			.append (SharedConstants::newline)
			.append ("  ").append (arm64_gas_name.data ()).append (SharedConstants::newline)
			.append ("  ").append (arm32_gas_name.data ()).append (SharedConstants::newline)
			.append ("  ").append (x86_gas_name.data ()).append (SharedConstants::newline)
			.append ("  ").append (x64_gas_name.data ()).append (SharedConstants::newline);
		return usage (true /* is_error */, message);
	} else {
		std::string message { "Unknown program name '" };
		message.append (arch_name).append ("'").append (SharedConstants::newline);
		return usage (true /* is_error */, message);
	}

	auto&& [terminate, is_error] = parse_arguments (argc, argv, mc_runner);
	if (terminate || is_error) {
		return is_error ? SharedConstants::wrapper_general_error_code : 0;
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

		std::cout << "Have multiple output files:" << SharedConstants::newline;
		for (fs::path const& output : output_files) {
			std::cout << "  " << output << SharedConstants::newline;
			ld->append_program_argument (output.string ());
		}

		return ld->run ();
	}

	return 0;
}

ParseArgsResult Gas::parse_arguments (int argc, char **argv, std::unique_ptr<LlvmMcRunner>& mc_runner)
{
	cxxopts::Options options = create_options ();

	options.parse_positional ("asmfile");
	auto result = options.parse(argc, argv);
	if (result.count ("help") > 0) {
		exit (usage (false /* is_error */));
	}

	bool is_error = false;
	bool show_version = false;

	if (result.count ("version") > 0) {
		show_version = true;
	}

	if (result.count ("V") > 0) {
		show_version = true;
	}

	if (show_version) {
		std::cout << program_name () << " v" << XA_UTILS_VERSION << ", " << PROGRAM_DESCRIPTION << SharedConstants::newline
		          << "\tGAS version compatibility: " << BINUTILS_VERSION << SharedConstants::newline
		          << "\tllvm-mc version compatibility: " << LLVM_VERSION << SharedConstants::newline << SharedConstants::newline;
		return {true /* terminate */, false /* is_error */};
	}

	if (result.count ("gen-debug") > 0) {
		mc_runner->generate_debug_info ();
	}

	if (result.count ("mfpu") > 0) {
		mc_runner->map_option ("mfpu", result["mfpu"].as<std::string> ());
	}

	if (result.count ("o") > 0) {
		_gas_output_file = result["o"].as<std::string> ();
	}

	if (_gas_output_file.empty ()) {
		_gas_output_file = Constants::default_output_name;
	}

	if (result.count ("asmfile") > 0) {
		for (std::string const& asmfile : result["asmfile"].as<std::vector<std::string>> ()) {
			// Ignore the arch hack parameter
			if (!asmfile.starts_with (Constants::arch_hack_param)) {
				input_files.emplace_back (asmfile);
			}
		}
	}

 	return {false /* terminate */, is_error};
}
