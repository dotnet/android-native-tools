// SPDX-License-Identifier: MIT
#include "gas.hh"

xamarin::android::gas::Gas app;

// const char *get_program_name ()
// {
// 	return app.program_name ();
// }

int main (int argc, char **argv)
{
	return app.run (argc, argv);
}
