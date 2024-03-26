// SPDX-License-Identifier: MIT
#if !defined(GAS_PLATFORM_HH)
#define GAS_PLATFORM_HH

#include <string>
#include <iostream>

#if defined(_WIN32)
#define STDOUT std::cout
#define STDERR std::cerr
#define PSTR(_str_lit_) (_str_lit_)
#define PCHAR(_ch_) (_ch_)
#else
#define STDOUT std::cout
#define STDERR std::cerr
#define PSTR(_str_lit_) (_str_lit_)
#define PCHAR(_ch_) (_ch_)
#endif

namespace platform {
#if !defined (_WIN32)
	using string = std::string;
	using string_view = std::string_view;
#else
	using string = std::string;
	using string_view = std::string_view;
#endif
}

#endif // ndef GAS_PLATFORM_HH
