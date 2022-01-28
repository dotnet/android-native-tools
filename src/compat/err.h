// Dear Emacs, this is a -*- C++ -*- source
// SPDX-License-Identifier: MIT
#if !defined (__COMPAT_ERR_H)
#define __COMPAT_ERR_H

#include <stdarg.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

extern void warnx (const char *__format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern void vwarnx (const char *__format, va_list) __attribute__ ((__format__ (__printf__, 1, 0)));

#if __cplusplus
}
#endif // __cplusplus

#endif // !__COMPAT_ERR_H
