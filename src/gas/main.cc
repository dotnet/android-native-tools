// SPDX-License-Identifier: MIT
#include <iostream>
#include "gas.hh"

int main (int argc, char **argv)
{
	xamarin::android::gas::Gas app;

	// On windows this obtains the utf8 version of args
	app.get_command_line (argc, argv);
	std::cout << "Command line converted" << std::endl;
	// TODO: handle exceptions here (use backward for stacktrace perhaps?)
	return app.run (argc, argv);
}
