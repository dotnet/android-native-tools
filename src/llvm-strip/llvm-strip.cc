// SPDX-License-Identifier: MIT
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include "../shared/process.hh"
#include "../shared/shared-constants.hh"
#include "llvm-strip.hh"

using namespace xamarin::android::binutils;
using namespace xamarin::android::llvm_strip;
namespace fs = std::filesystem;

namespace {
	class Option
	{
		static constexpr std::string_view short_arg_mark { "-" };
		static constexpr std::string_view long_arg_mark { "--" };

	public:
		consteval Option (const std::string_view _parse, const std::string_view _check = "", const std::string_view _target = "")
			: parse_option (_parse),
			  check_option (select (_check, _parse)),
			  target_option (_target.empty() ? select (_check, _parse) : _target)
		{}

		constexpr const std::string_view::value_type* parse () const
		{
			return parse_option.data ();
		}

		constexpr const std::string_view::value_type* check () const
		{
			return check_option.data ();
		}

		std::string target_arg () const
		{
			std::string arg { target_option.length () == 1 ? short_arg_mark : long_arg_mark };
			arg.append (target_option);
			return arg;
		}

	private:
		const std::string_view parse_option;
		const std::string_view check_option;
		const std::string_view target_option;

	private:
		static consteval std::string_view select (std::string_view const& alt_one, std::string_view const& alt_two)
		{
			if (!alt_one.empty()) {
				return alt_one;
			}

			if (!alt_two.empty ()) {
				return alt_two;
			}

			return "";
		}
	};

	constexpr std::string_view GNU_GROUP                { "GNU" };

	constexpr Option GNU_OPT_INFO                       { "info" };
	constexpr Option GNU_OPT_INPUT_TARGET               { "I,input-target", "input-target" };
	constexpr Option GNU_OPT_KEEP_SECTION_SYMBOLS       { "keep-section-symbols" };
	constexpr Option GNU_OPT_MERGE_NOTES                { "M,merge-notes", "merge-notes" };
	constexpr Option GNU_OPT_NO_MERGE_NOTES             { "no-merge-notes" };
	constexpr Option GNU_OPT_OUTPUT_TARGET              { "O,output-target", "output-target" };
	constexpr Option GNU_OPT_REMOVE_RELOCATIONS         { "remove-relocations" };
	constexpr Option GNU_OPT_STRIP_DWO                  { "strip-dwo" };
	constexpr Option GNU_OPT_STRIP_SECTION_HEADERS      { "strip-section-headers" };
	constexpr Option GNU_OPT_TARGET                     { "F,target", "target" };
	constexpr Option GNU_OPT_VERBOSE                    { "v,verbose", "verbose" };

	constexpr Option LLVM_OPT_ALLOW_BROKEN_LINKS        { "allow-broken-links" };
	constexpr Option LLVM_OPT_NO_STRIP_ALL              { "no-strip-all" };
	constexpr Option LLVM_OPT_REGEX                     { "regex" };
	constexpr Option LLVM_OPT_STRIP_ALL_GNU             { "strip-all-gnu" };
	constexpr Option LLVM_OPT_T                         { "T" };

	constexpr Option OPT_D                              { "d", "", "strip-debug" };
	constexpr Option OPT_DISABLE_DETERMINISTIC_ARCHIVES { "U,disable-deterministic-archives", "disable-deterministic-archives" };
	constexpr Option OPT_DISCARD_ALL                    { "x,discard-all", "discard-all" };
	constexpr Option OPT_DISCARD_LOCALS                 { "X,discard-locals", "discard-locals" };
	constexpr Option OPT_ENABLE_DETERMINISTIC_ARCHIVES  { "D,enable-deterministic-archives", "enable-deterministic-archives" };
	constexpr Option OPT_G                              { "g", "", "strip-debug" };
	constexpr Option OPT_HELP                           { "h,help", "help" };
	constexpr Option OPT_KEEP_FILE_SYMBOLS              { "keep-file-symbols" };
	constexpr Option OPT_KEEP_SYMBOL                    { "K,keep-symbol", "keep-symbol" };
	constexpr Option OPT_O                              { "o" };
	constexpr Option OPT_ONLY_KEEP_DEBUG                { "only-keep-debug" };
	constexpr Option OPT_PRESERVE_DATES                 { "p,preserve-dates", "preserve-dates" };
	constexpr Option OPT_REMOVE_SECTION                 { "R,remove-section", "remove-section" };
	constexpr Option OPT_CAPITAL_S                      { "S", "", "strip-debug" };
	constexpr Option OPT_STRIP_ALL                      { "s,strip-all", "strip-all" };
	constexpr Option OPT_STRIP_DEBUG                    { "strip-debug" };
	constexpr Option OPT_STRIP_SYMBOL                   { "N,strip-symbol", "strip-symbol" };
	constexpr Option OPT_STRIP_UNNEEDED                 { "strip-unneeded" };
	constexpr Option OPT_VERSION                        { "V,version", "version" };
	constexpr Option OPT_WILDCARD                       { "w,wildcard", "wildcard" };

	constexpr Option OPT_INPUTS                         { "inputs..." };
	constexpr Option OPT_WRAPPER_VERSION                { "wrapper-version" };

#if defined(_WIN32)
	constexpr std::string_view LLVM_OBJCOPY_NAME { "llvm-objcopy.exe" };
#else
	constexpr std::string_view LLVM_OBJCOPY_NAME { "llvm-objcopy" };
#endif
}

cxxopts::Options LlvmStrip::create_options ()
{
	cxxopts::Options options (program_name(), PROGRAM_DESCRIPTION.data());

	options
		.positional_help (OPT_INPUTS.parse())
		.set_width (80);

	options.add_options ("GNU strip")
		(GNU_OPT_TARGET.parse(),                "Set both input and output format to <arg>", cxxopts::value<std::string>())
		(GNU_OPT_INPUT_TARGET.parse(),          "Assume input file is in format <arg>", cxxopts::value<std::string>())
		(GNU_OPT_MERGE_NOTES.parse(),           "Remove redundant entries in note sections (default)")
		(GNU_OPT_NO_MERGE_NOTES.parse(),        "Do not attempt to remove redundant notes")
		(GNU_OPT_OUTPUT_TARGET.parse(),         "Create an output file in format <arg>", cxxopts::value<std::string>())
		(GNU_OPT_INFO.parse(),                  "List object formats & architectures supported")
		(GNU_OPT_KEEP_SECTION_SYMBOLS.parse(),  "Do not strip section symbols")
		(GNU_OPT_REMOVE_RELOCATIONS.parse(),    "Remove relocations from section <arg>", cxxopts::value<std::string>())
		(GNU_OPT_STRIP_DWO.parse(),             "Remove all DWO sections")
		(GNU_OPT_STRIP_SECTION_HEADERS.parse(), "Strip section headers from the output")
		(GNU_OPT_VERBOSE.parse(),               "List all object files modified");

	options.add_options ("llvm-strip")
		(LLVM_OPT_T.parse(),                  "Remove Swift symbols")
		(LLVM_OPT_ALLOW_BROKEN_LINKS.parse(), "Allow the tool to remove sections even if it would leave invalid section references. The appropriate sh_link fields will be set to zero.")
		(LLVM_OPT_NO_STRIP_ALL.parse(),       "Disable --strip-all")
		(LLVM_OPT_REGEX.parse(),              "Permit regular expressions in name comparison")
		(LLVM_OPT_STRIP_ALL_GNU.parse(),      "Compatible with GNU's --strip-all");

	options.add_options ("Common LLVM and GNU strip")
		(OPT_ENABLE_DETERMINISTIC_ARCHIVES.parse(),  "Enable deterministic mode when operating on archives (use zero for UIDs, GIDs, and timestamps).")
		(OPT_DISABLE_DETERMINISTIC_ARCHIVES.parse(), "Disable -D behavior")
		(OPT_KEEP_SYMBOL.parse(),                    "Do not strip symbol <arg>", cxxopts::value<std::string>())
		(OPT_STRIP_SYMBOL.parse(),                   "Do not copy symbol <arg>", cxxopts::value<std::string>())
		(OPT_REMOVE_SECTION.parse(),                 "Also remove section <arg> from the output", cxxopts::value<std::string>())
		(OPT_VERSION.parse(),                        "Display this program's version number")
		(OPT_DISCARD_LOCALS.parse(),                 "Remove any compiler-generated symbols")
		(OPT_STRIP_DEBUG.parse(),                    "Remove all debugging symbols & sections")
		(OPT_D.parse(),                              "Alias for --strip-debug")
		(OPT_G.parse(),                              "Alias for --strip-debug")
		(OPT_CAPITAL_S.parse(),                      "Alias for --strip-debug")
		(OPT_KEEP_FILE_SYMBOLS.parse(),              "Do not strip file symbol(s)")
		(OPT_O.parse(),                              "Place stripped output into <arg>", cxxopts::value<std::string>())
		(OPT_ONLY_KEEP_DEBUG.parse(),                "Produce a debug file as the output that only preserves contents of sections useful for debugging purposes")
		(OPT_PRESERVE_DATES.parse(),                 "Copy modified/access timestamps to the output")
		(OPT_STRIP_ALL.parse(),                      "Remove all symbol and relocation information")
		(OPT_STRIP_UNNEEDED.parse(),                 "Remove all symbols not needed by relocations")
		(OPT_WILDCARD.parse(),                       "Permit wildcard in symbol comparison")
		(OPT_DISCARD_ALL.parse(),                    "Remove all non-global symbols")
		(OPT_HELP.parse(),                           "Display this output");

	options.add_options ("Wrapper")
		(OPT_WRAPPER_VERSION.parse(), "Show wrapper version information")
		(OPT_INPUTS.parse(),          "Input file(s)", cxxopts::value<std::vector<std::string>>());

	options.parse_positional (OPT_INPUTS.parse());
	return options;
}

ParseArgsResult LlvmStrip::parse_arguments (int argc, char **argv, Process &llvm_objcopy)
{
	cxxopts::Options options = create_options ();
	auto result = options.parse(argc, argv);

	auto have_option = [&] (Option const& option) -> bool {
		return result.count (option.check()) > 0;
	};

	if (have_option (OPT_HELP)) {
		exit (usage (false /* is_error */));
	}

	if (have_option (OPT_WRAPPER_VERSION)) {
		std::cout << program_name() << " v" << XA_UTILS_VERSION << ", " << PROGRAM_DESCRIPTION << SharedConstants::newline
		          << "\tGNU strip version compatibility: " << BINUTILS_VERSION << SharedConstants::newline
		          << "\tllvm-strip version compatibility: " << LLVM_VERSION << SharedConstants::newline << SharedConstants::newline;
		return {true /* terminate */, false /* is_error */};
	}

	auto check_unsupported = [&] (std::string_view const& group_name, Option const& option) {
		if (!have_option (option)) {
			return;
		}

		std::cerr << "Warning: " << group_name << " option '" << option.parse() << "' is unsupported." << SharedConstants::newline;
	};

	// First flag all the unsupported GNU options
	check_unsupported (GNU_GROUP, GNU_OPT_TARGET);
	check_unsupported (GNU_GROUP, GNU_OPT_INPUT_TARGET);
	check_unsupported (GNU_GROUP, GNU_OPT_MERGE_NOTES);
	check_unsupported (GNU_GROUP, GNU_OPT_NO_MERGE_NOTES);
	check_unsupported (GNU_GROUP, GNU_OPT_INPUT_TARGET);
	check_unsupported (GNU_GROUP, GNU_OPT_INFO);
	check_unsupported (GNU_GROUP, GNU_OPT_KEEP_SECTION_SYMBOLS);
	check_unsupported (GNU_GROUP, GNU_OPT_REMOVE_RELOCATIONS);
	check_unsupported (GNU_GROUP, GNU_OPT_STRIP_DWO);
	check_unsupported (GNU_GROUP, GNU_OPT_STRIP_SECTION_HEADERS);
	check_unsupported (GNU_GROUP, GNU_OPT_VERBOSE);

	auto option_data = [&]<typename T> (Option const& option, T &target) {
		target = result[option.check()].as<T>();
	};

	bool require_inputs = true;

	// LLVM options are passed verbatim
	if (have_option (LLVM_OPT_T)) {
		llvm_objcopy.append_program_argument (LLVM_OPT_T.target_arg());
	}

	if (have_option (LLVM_OPT_ALLOW_BROKEN_LINKS)) {
		llvm_objcopy.append_program_argument (LLVM_OPT_ALLOW_BROKEN_LINKS.target_arg());
	}

	if (have_option (LLVM_OPT_NO_STRIP_ALL)) {
		llvm_objcopy.append_program_argument (LLVM_OPT_NO_STRIP_ALL.target_arg());
	}

	if (have_option (LLVM_OPT_REGEX)) {
		llvm_objcopy.append_program_argument (LLVM_OPT_REGEX.target_arg());
	}

	if (have_option (LLVM_OPT_STRIP_ALL_GNU)) {
		llvm_objcopy.append_program_argument (LLVM_OPT_STRIP_ALL_GNU.target_arg());
	}

	if (have_option (OPT_ENABLE_DETERMINISTIC_ARCHIVES)) {
		llvm_objcopy.append_program_argument (OPT_ENABLE_DETERMINISTIC_ARCHIVES.target_arg());
	}

	if (have_option (OPT_DISABLE_DETERMINISTIC_ARCHIVES)) {
		llvm_objcopy.append_program_argument (OPT_DISABLE_DETERMINISTIC_ARCHIVES.target_arg());
	}

	std::string option_value;
	if (have_option (OPT_KEEP_SYMBOL)) {
		option_data (OPT_KEEP_SYMBOL, option_value);
		llvm_objcopy.append_program_argument (OPT_KEEP_SYMBOL.target_arg(), option_value);
	}

	if (have_option (OPT_STRIP_SYMBOL)) {
		option_data (OPT_STRIP_SYMBOL, option_value);
		llvm_objcopy.append_program_argument (OPT_STRIP_SYMBOL.target_arg(), option_value);
	}

	if (have_option (OPT_REMOVE_SECTION)) {
		option_data (OPT_REMOVE_SECTION, option_value);
		llvm_objcopy.append_program_argument (OPT_REMOVE_SECTION.target_arg(), option_value);
	}

	if (have_option (OPT_VERSION)) {
		llvm_objcopy.append_program_argument (OPT_VERSION.target_arg ());
		require_inputs = false;
	}

	if (have_option (OPT_DISCARD_LOCALS)) {
		llvm_objcopy.append_program_argument (OPT_DISCARD_LOCALS.target_arg ());
	}

	if (have_option (OPT_STRIP_DEBUG) || have_option (OPT_D) || have_option (OPT_G) || have_option (OPT_CAPITAL_S)) {
		llvm_objcopy.append_program_argument (OPT_STRIP_DEBUG.target_arg ());
	}

	if (have_option (OPT_KEEP_FILE_SYMBOLS)) {
		llvm_objcopy.append_program_argument (OPT_KEEP_FILE_SYMBOLS.target_arg());
	}

	if (have_option (OPT_O)) {
		option_data (OPT_O, option_value);
		llvm_objcopy.append_program_argument (OPT_O.target_arg (), option_value);
	}

	if (have_option (OPT_ONLY_KEEP_DEBUG)) {
		llvm_objcopy.append_program_argument (OPT_ONLY_KEEP_DEBUG.target_arg ());
	}

	if (have_option (OPT_PRESERVE_DATES)) {
		llvm_objcopy.append_program_argument (OPT_PRESERVE_DATES.target_arg());
	}

	if (have_option (OPT_STRIP_ALL)) {
		llvm_objcopy.append_program_argument (OPT_STRIP_ALL.target_arg());
	}

	if (have_option (OPT_STRIP_UNNEEDED)) {
		llvm_objcopy.append_program_argument (OPT_STRIP_UNNEEDED.target_arg());
	}

	if (have_option (OPT_WILDCARD)) {
		llvm_objcopy.append_program_argument (OPT_WILDCARD.target_arg());
	}

	if (have_option (OPT_DISCARD_ALL)) {
		llvm_objcopy.append_program_argument (OPT_DISCARD_ALL.target_arg ());
	}

	if (!require_inputs) {
		return {false /* terminate */, false /* is_error */};
	}

	std::vector<std::string> input_files;
	if (have_option (OPT_INPUTS)) {
		option_data (OPT_INPUTS, input_files);
	}

	if (input_files.empty()) {
		std::cerr << "At least a single input file must be specified on the command line." << SharedConstants::newline;
		return {true /* terminate */, true /* is_error */};
	}

	for (std::string &file : input_files) {
		llvm_objcopy.append_program_argument (file);
	}

	return {false /* terminate */, false /* is_error */};
}

int LlvmStrip::run (int argc, char **argv)
{
	determine_program_dir (argc, argv);
	fs::path llvm_objcopy_path = program_dir () / LLVM_OBJCOPY_NAME;
	Process llvm_objcopy { llvm_objcopy_path };

	auto&& [terminate, is_error] = parse_arguments (argc, argv, llvm_objcopy);
	if (terminate || is_error) {
		return is_error ? SharedConstants::wrapper_general_error_code : 0;
	}

	return llvm_objcopy.run ();
}
