// SPDX-License-Identifier: MIT
#include <iostream>

#include <cxxopts.hpp>

#include "../shared/shared-constants.hh"
#include "llvm-strip.hh"

using namespace xamarin::android::binutils;
using namespace xamarin::android::llvm_strip;

namespace {
	constexpr std::string_view GNU_GROUP                     = "GNU";
	constexpr std::string_view GNU_OPT_TARGET                = "F,target";
	constexpr std::string_view GNU_OPT_INPUT_TARGET          = "I,input-target";
	constexpr std::string_view GNU_OPT_MERGE_NOTES           = "M,merge-notes";
	constexpr std::string_view GNU_OPT_NO_MERGE_NOTES        = "no-merge-notes";
	constexpr std::string_view GNU_OPT_OUTPUT_TARGET         = "O,output-target";
	constexpr std::string_view GNU_OPT_INFO                  = "info";
	constexpr std::string_view GNU_OPT_KEEP_SECTION_SYMBOLS  = "keep-section-symbols";
	constexpr std::string_view GNU_OPT_REMOVE_RELOCATIONS    = "remove-relocations";
	constexpr std::string_view GNU_OPT_STRIP_DWO             = "strip-dwo";
	constexpr std::string_view GNU_OPT_STRIP_SECTION_HEADERS = "strip-section-headers";
	constexpr std::string_view GNU_OPT_VERBOSE               = "v,verbose";

	constexpr std::string_view LLVM_OPT_T                  = "T";
	constexpr std::string_view LLVM_OPT_ALLOW_BROKEN_LINKS = "allow-broken-links";
	constexpr std::string_view LLVM_OPT_NO_STRIP_ALL       = "no-strip-all";
	constexpr std::string_view LLVM_OPT_REGEX              = "regex";
	constexpr std::string_view LLVM_OPT_STRIP_ALL_GNU      = "strip-all-gnu";
}

cxxopts::Options LlvmStrip::create_options ()
{
	cxxopts::Options options (program_name(), PROGRAM_DESCRIPTION.data());

	options
		.positional_help ("inputs...")
		.set_width (80);

	options.add_options ("GNU strip")
		(GNU_OPT_TARGET.data(),                "Set both input and output format to <arg>", cxxopts::value<std::string>())
		(GNU_OPT_INPUT_TARGET.data(),          "Assume input file is in format <arg>", cxxopts::value<std::string>())
		(GNU_OPT_MERGE_NOTES.data(),           "Remove redundant entries in note sections (default)")
		(GNU_OPT_NO_MERGE_NOTES.data(),        "Do not attempt to remove redundant notes")
		(GNU_OPT_OUTPUT_TARGET.data(),         "Create an output file in format <arg>", cxxopts::value<std::string>())
		(GNU_OPT_INFO.data(),                  "List object formats & architectures supported")
		(GNU_OPT_KEEP_SECTION_SYMBOLS.data(),  "Do not strip section symbols")
		(GNU_OPT_REMOVE_RELOCATIONS.data(),    "Remove relocations from section <arg>", cxxopts::value<std::string>())
		(GNU_OPT_STRIP_DWO.data(),             "Remove all DWO sections")
		(GNU_OPT_STRIP_SECTION_HEADERS.data(), "Strip section headers from the output")
		(GNU_OPT_VERBOSE.data(),               "List all object files modified");

	options.add_options ("llvm-strip")
		(LLVM_OPT_T.data(),                  "Remove Swift symbols")
		(LLVM_OPT_ALLOW_BROKEN_LINKS.data(), "Allow the tool to remove sections even if it would leave invalid section references. The appropriate sh_link fields will be set to zero.")
		(LLVM_OPT_NO_STRIP_ALL.data(),       "Disable --strip-all")
		(LLVM_OPT_REGEX.data(),              "Permit regular expressions in name comparison")
		(LLVM_OPT_STRIP_ALL_GNU.data(),      "Compatible with GNU's --strip-all");

	options.add_options ("Common LLVM and GNU strip")
		("D,enable-deterministic-archives", "Enable deterministic mode when operating on archives (use zero for UIDs, GIDs, and timestamps).")
		("U,disable-deterministic-archives", "Disable -D behavior")
		("K,keep-symbol", "Do not strip symbol <arg>", cxxopts::value<std::string>())
		("N,strip-symbol", "Do not copy symbol <arg>", cxxopts::value<std::string>())
		("R,remove-section", "Also remove section <name> from the output", cxxopts::value<std::string>())
		("V,version", "Display this program's version number")
		("X,discard-locals", "Remove any compiler-generated symbols")
		("strip-debug", "Remove all debugging symbols & sections")
		("d", "Alias for --strip-debug")
		("g", "Alias for --strip-debug")
		("S", "Alias for --strip-debug")
		("keep-file-symbols", "Do not strip file symbol(s)")
		("o", "Place stripped output into <arg>", cxxopts::value<std::string>())
		("only-keep-debug", "Produce a debug file as the output that only preserves contents of sections useful for debugging purposes")
		("p,preserve-dates", "Copy modified/access timestamps to the output")
		("s,strip-all", "Remove all symbol and relocation information")
		("strip-unneeded", "Remove all symbols not needed by relocations")
		("w,wildcard", "Permit wildcard in symbol comparison")
		("x,discard-all", "Remove all non-global symbols")
		("h,help", "Display this output");

	options.add_options ("Wrapper")
		("wrapper-version", "Show wrapper version information");

	return options;
}

ParseArgsResult LlvmStrip::parse_arguments (int argc, char **argv)
{
	cxxopts::Options options = create_options ();
	options.parse_positional ("inputs...");

	auto result = options.parse(argc, argv);
	if (result.count ("help") > 0) {
		exit (usage (false /* is_error */));
	}

	if (result.count ("wrapper-version") > 0) {
		std::cout << program_name() << " v" << XA_UTILS_VERSION << ", " << PROGRAM_DESCRIPTION << SharedConstants::newline
		          << "\tGNU strip version compatibility: " << BINUTILS_VERSION << SharedConstants::newline
		          << "\tllvm-strip version compatibility: " << LLVM_VERSION << SharedConstants::newline << SharedConstants::newline;
		return {true /* terminate */, false /* is_error */};
	}

	bool have_unsupported = false;
	auto check_unsupported = [&] (std::string_view const& group_name, std::string_view const& option_name) {
		if (result.count (option_name.data()) == 0) {
			return;
		}

		std::cerr << group_name << " option '" << option_name << "' is unsupported." << SharedConstants::newline;
		have_unsupported = true;
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

	if (have_unsupported) {
		return {true /* terminate */, true /* is_error */};
	}

	auto have_option = [&] (std::string_view const& option_name) {

	};

	// LLVM options are passed verbatim

	return {false /* terminate */, false /* is_error */};
}

int LlvmStrip::run (int argc, char **argv)
{
	determine_program_dir (argc, argv);

	auto&& [terminate, is_error] = parse_arguments (argc, argv);
	if (terminate || is_error) {
		return is_error ? SharedConstants::wrapper_general_error_code : 0;
	}

	return 0;
}
