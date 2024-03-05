// SPDX-License-Identifier: MIT
#include <filesystem>

#include <cxxopts.hpp>

#include "../shared/process.hh"
#include "ld.hh"

using namespace xamarin::android::binutils;
using namespace xamarin::android::ld;
namespace fs = std::filesystem;

cxxopts::Options LD::create_options ()
{
	return cxxopts::Options (program_name(), PROGRAM_DESCRIPTION.data());
}

int LD::run (int argc, char **argv)
{
	determine_program_dir (argc, argv);
	fs::path llvm_lld_path = program_dir () / LLVM_LLD_NAME;
	Process llvm_lld { llvm_lld_path, LLVM_LLD_INVOKE_NAME };

	// This wrapper simply passes all the arguments along
	for (int i = 1; i < argc; i++) {
		llvm_lld.append_program_argument (argv[i]);
	}

	return llvm_lld.run ();
}
