// SPDX-License-Identifier: MIT
#include "llvm-strip.hh"

int main (int argc, char **argv)
{
	xamarin::android::llvm_strip::LlvmStrip app;

	// On windows this obtains the utf8 version of args
	app.get_command_line (argc, argv);
	return app.run (argc, argv);
}
