// SPDX-License-Identifier: MIT
#include "err.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern const char *get_program_name ();

extern "C" void
warnx (const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	vwarnx (fmt, ap);
	va_end (ap);
}

extern "C" void
vwarnx (const char *fmt, va_list ap)
{
	fprintf (stderr, "%s: ", get_program_name ());

	if (fmt != nullptr) {
		vfprintf (stderr, fmt, ap);
	}

	fprintf (stderr, "\n");
}
