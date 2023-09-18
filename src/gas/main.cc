// SPDX-License-Identifier: MIT
#include "gas.hh"

xamarin::android::gas::Gas app;

int main (int argc, char **argv)
{
	// On windows this obtains the utf8 version of args
	app.get_command_line (argc, argv);
	// TODO: handle exceptions here (use backward for stacktrace perhaps?)
	return app.run (argc, argv);
}
