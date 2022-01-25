// SPDX-License-Identifier: MIT
#if !defined (__EXCEPTIONS_HH)
#define __EXCEPTIONS_HH

#include <stdexcept>

namespace xamarin::android::gas
{
	class invalid_argument_error : public std::invalid_argument
	{
	public:
		explicit invalid_argument_error (const std::string& what_arg)
			: std::invalid_argument (what_arg)
		{}

		explicit invalid_argument_error (const char* what_arg)
			: std::invalid_argument (what_arg)
		{}
	};

	class invalid_operation_error : public std::logic_error
	{
	public:
		explicit invalid_operation_error (const std::string& what_arg)
			: logic_error (what_arg)
		{}

		explicit invalid_operation_error (const char* what_arg)
			: logic_error (what_arg)
		{}
	};
}
#endif
