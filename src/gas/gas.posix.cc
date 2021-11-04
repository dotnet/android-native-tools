// SPDX-License-Identifier: MIT
#include <libgen.h>
#include <cstring>

#include "gas.hh"

using namespace xamarin::android::gas;

void Gas::init_platform ()
{
	newline = "\n";
}

void Gas::determine_program_name ([[maybe_unused]] int argc, char **argv)
{
	_program_name = strdup (basename (argv[0]));
}
