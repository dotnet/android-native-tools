// SPDX-License-Identifier: MIT
#include "gas.hh"

xamarin::android::gas::Gas app;

int main (int argc, char **argv)
{
	// TODO: handle exceptions here (use backward for stacktrace perhaps?)
	return app.run (argc, argv);
}
