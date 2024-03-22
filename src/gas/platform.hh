// SPDX-License-Identifier: MIT
#if !defined(GAS_PLATFORM_HH)
#define GAS_PLATFORM_HH

#include <string>
#include <iostream>

#if defined(_WIN32)
#define STDOUT std::wcout
#define STDERR std::wcerr
#define PSTR(_str_lit_) (L##_str_lit_)
#else
#define STDOUT std::cout
#define STDERR std::cerr
#define PSTR(_str_lit_) (_str_lit_)
#endif

namespace platform {
#if !defined (_WIN32)
	using string = std::string;
	using string_view = std::string_view;
#else
	using string = std::wstring;
	using string_view = std::wstring_view;
#endif
}

#endif // ndef GAS_PLATFORM_HH
