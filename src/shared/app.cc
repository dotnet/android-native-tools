// SPDX-License-Identifier: MIT
#include <iostream>

#include <cxxopts.hpp>

#include "app.hh"
#include "shared-constants.hh"

using namespace xamarin::android::binutils;

int App::usage (bool is_error, std::string const message)
{
	if (!message.empty ()) {
		std::cerr << message << SharedConstants::newline << SharedConstants::newline;
	}

	cxxopts::Options options = create_options ();
	std::cerr << options.help () << std::endl << std::endl;
	usage_print_extra (is_error);

	return is_error ? 1 : 0;
}
