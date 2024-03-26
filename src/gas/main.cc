// SPDX-License-Identifier: MIT
#include <iostream>
#include <vector>

#include "gas.hh"
#include "platform.hh"

int main (int argc, char **argv)
{
	xamarin::android::gas::Gas app;

	std::vector<platform::string> args = app.get_command_line (argc, argv);

	// TODO: handle exceptions here (use backward for stacktrace perhaps?)
	return app.run (args);
}
