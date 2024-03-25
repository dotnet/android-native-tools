// SPDX-License-Identifier: MIT
#if !defined (_WIN32)
#include <unistd.h>
#endif

#include <array>
#include <cstring>
#include <iostream>
#include <filesystem>

#include "command_line.hh"
#include "constants.hh"
#include "gas.hh"
#include "llvm_mc_runner.hh"

using namespace xamarin::android::gas;

int Gas::usage (bool is_error, platform::string const message)
{
	if (!message.empty ()) {
		STDERR << message << Constants::newline << Constants::newline;
	}

	STDERR << "`" << program_name () << "` takes a subset of arguments accepted by the GNU Assembler (gas) program." << Constants::newline
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

int Gas::run (std::vector<platform::string> args)
{
	STDOUT << "Gas::run" << std::endl;
	determine_program_dir (args);
	STDOUT << "Program dir determined" << std::endl;

	auto lowercase_string = [](platform::string& s) {
		std::transform (
			s.begin (),
			s.end (),
			s.begin (),
			[](platform::string::value_type c) { return std::tolower(c); }
		);
	};

	STDOUT << "Determining arch name" << std::endl;
	platform::string arch_name { generic_gas_name };
	platform::string first_param { args.size () > 1 ? args[1] : PSTR("") };
	if (!first_param.empty () && first_param.length () > Constants::arch_hack_param.size () && first_param.find (Constants::arch_hack_param) == 0) {
		arch_name = first_param.substr (Constants::arch_hack_param.size ());
		lowercase_string (arch_name);
	}
	STDOUT << "arch_name == " << arch_name << std::endl;
	_program_name = arch_name;

	std::unique_ptr<LlvmMcRunner> mc_runner;
	platform::string ld_name;
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
		platform::string message { PSTR("Program invoked via its generic name (") };
		message
			.append (generic_gas_name)
			.append (PSTR("), please use one of the ABI-prefixed names:"))
			.append (Constants::newline)
			.append (PSTR("  ")).append (arm64_gas_name.data ()).append (Constants::newline)
			.append (PSTR("  ")).append (arm32_gas_name.data ()).append (Constants::newline)
			.append (PSTR("  ")).append (x86_gas_name.data ()).append (Constants::newline)
			.append (PSTR("  ")).append (x64_gas_name.data ()).append (Constants::newline);
		return usage (true /* is_error */, message);
	} else {
		platform::string message { PSTR("Unknown program name '") };
		message.append (arch_name).append (PSTR("'")).append (Constants::newline);
		return usage (true /* is_error */, message);
	}

	STDOUT << "About to parse arguments" << std::endl;
	auto&& [terminate, is_error] = parse_arguments (args, mc_runner);
	STDOUT << "Arguments parsed; terminate == " << terminate << "; is_error == " << is_error << std::endl;
	if (terminate || is_error) {
		return is_error ? Constants::wrapper_general_error_code : 0;
	}

	STDOUT << "Getting path to llvm_mc" << std::endl;
	fs::path llvm_mc = program_dir () / Constants::llvm_mc_name;
	STDOUT << "llvm_mc == " << llvm_mc.c_str () << std::endl;

	bool multiple_input_files = false;
	bool derive_output_file_name = false;
	switch (input_files.size ()) {
		case 0:
			return usage (true, PSTR("missing input files on command line"));

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
		STDOUT << "running llvm-mc for '" << input.c_str () << std::endl;
		mc_runner->set_input_file_path (input, derive_output_file_name);
		int ret = mc_runner->run (llvm_mc);
		if (ret != 0) {
			STDOUT << "  mc_runner failed with " << ret << std::endl;
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
		ld->append_program_argument (PSTR("-o"));
		ld->append_program_argument (_gas_output_file.empty () ? platform::string (Constants::default_output_name) : _gas_output_file.native ());
		ld->append_program_argument (PSTR("--relocatable"));

		STDOUT << "Have multiple output files:" << Constants::newline;
		for (fs::path const& output : output_files) {
			STDOUT << "  " << output << Constants::newline;
			ld->append_program_argument (output.native ());
		}

		return ld->run ();
	}

	return 0;
}

constexpr std::array<CommandLineOption, 21> all_options {{
	// Arguments ignored by GAS, we shall ignore them silently too
	{ CLIPARAM("divide"),    OptionId::Ignore },
	{ CLIPARAM("k"),         OptionId::Ignore },
	{ CLIPARAM("nocpp"),     OptionId::Ignore },
	{ CLIPARAM("Qn"),        OptionId::Ignore },
	{ CLIPARAM("Qy"),        OptionId::Ignore },
	{ CLIPARAM("s"),         OptionId::Ignore },
	{ CLIPARAM("w"),         OptionId::Ignore },
	{ CLIPARAM("X"),         OptionId::Ignore },

	// Global GAS arguments we support
	{ CLIPARAM("o"),         OptionId::O,              ArgumentValue::Required },
	{ CLIPARAM("warn"),      OptionId::Warn },
	{ CLIPARAM("g"),         OptionId::G },
	{ CLIPARAM("gen-debug"), OptionId::G },

	// Arguments handled by us, not passed to llvm-mc
	{ CLIPARAM("h"),         OptionId::Help },
	{ CLIPARAM("help"),      OptionId::Help },
	{ CLIPARAM("V"),         OptionId::Version },
	{ CLIPARAM("version"),   OptionId::VersionExit },

	// x86 arguments
	{ CLIPARAM("32"),        OptionId::Ignore,         TargetArchitecture::X86 }, // llvm-mc doesn't need this
	{ CLIPARAM("64"),        OptionId::Ignore,         TargetArchitecture::X86 }, // llvm-mc doesn't need this

	// x64 arguments
	{ CLIPARAM("32"),        OptionId::Ignore,         TargetArchitecture::X64 }, // llvm-mc doesn't need this
	{ CLIPARAM("64"),        OptionId::Ignore,         TargetArchitecture::X64 }, // llvm-mc doesn't need this

	// Arm32 arguments
	{ CLIPARAM("mfpu"),      OptionId::MFPU,           ArgumentValue::Required, TargetArchitecture::ARM32 },
}};

Gas::ParseArgsResult Gas::parse_arguments (std::vector<platform::string> &args, std::unique_ptr<LlvmMcRunner>& mc_runner)
{
	bool terminate = false, is_error = false;
	bool show_version = false, show_help = false;

	auto handle_arg = [&](CommandLine::TCallbackOption option, CommandLine::TOptionValue val) {
		if (std::holds_alternative<uint32_t> (option)) {
			platform::string arg = std::get<platform::string> (val);
			// Positional argument
			STDOUT << "Positional argument no. " << std::get<uint32_t> (option) << ": " << arg << "\n";
			if (arg.starts_with (Constants::arch_hack_param)) {
				// Arch hack, ignore
				STDOUT << "  arch hack param, ignored\n";
				return;
			}

			STDOUT << "  input file\n";
			input_files.emplace_back (arg);
			return;
		}

		const auto opt = std::get<const CommandLineOption> (option);
		STDOUT << "Option argument: " << platform::string (opt.name) << "\n";
		if (opt.id == OptionId::Ignore) {
			STDOUT << "  ignored\n";
			return;
		}

		if (std::holds_alternative<bool> (val)) {
			STDOUT << "  boolean\n";
		} else {
			STDOUT << "  value: " << std::get<platform::string> (val) << "\n";
		}

		switch (opt.id) {
			case OptionId::Version:
				show_version = true;
				break;

			case OptionId::VersionExit:
				show_version = true;
				terminate = true;
				break;

			case OptionId::Help:
				show_help = true;
				terminate = true;
				break;

			case OptionId::O:
				_gas_output_file = std::get<platform::string> (val);
				break;

			case OptionId::MFPU:
				mc_runner->map_option (PSTR("mfpu"), std::get<platform::string> (val));
				break;

			case OptionId::G:
				mc_runner->generate_debug_info ();
				break;

			default:
				break;
		}
	};

	CommandLine cmdline { target_arch () };
	if (!cmdline.parse (all_options, args, handle_arg)) {
		terminate = true;
		is_error = true;
	}

	constexpr platform::string_view PROGRAM_DESCRIPTION { PSTR("Xamarin.Android GAS adapter for llvm-mc") };
	if (show_help) {
		usage (false /* is_error */);
		return {true, false};
	} else if (show_version) {
		STDOUT << program_name () << " v" << XA_UTILS_VERSION << ", " << PROGRAM_DESCRIPTION << Constants::newline
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
