// SPDX-License-Identifier: MIT
#include "ld.hh"

int main (int argc, char **argv)
{
	xamarin::android::ld::LD app;

	// On windows this obtains the utf8 version of args
	app.get_command_line (argc, argv);
	return app.run (argc, argv);
}
