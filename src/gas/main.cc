// SPDX-License-Identifier: MIT
#include <vector>

#include "gas.hh"
#include "platform.hh"

#if defined(_WIN32)
int wmain (int argc, wchar_t **argv)
#else
int main (int argc, char **argv)
#endif
{
	xamarin::android::gas::Gas::platform_setup ();
	xamarin::android::gas::Gas app;

	std::vector<platform::string> args = app.get_command_line (argc, argv);

	// TODO: handle exceptions here (use backward for stacktrace perhaps?)
	return app.run (args);
}
