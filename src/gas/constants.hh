// SPDX-License-Identifier: MIT
#if !defined (__CONSTANTS_HH)
#define __CONSTANTS_HH

#include "platform.hh"

namespace xamarin::android::gas
{
	class Constants final
	{
	public:
#if defined (_WIN32)
		static constexpr platform::string_view newline { PSTR("\r\n") };
		static constexpr platform::string_view llvm_mc_name { PSTR("llvm-mc.exe") };
#else
		static constexpr platform::string_view newline { "\n" };
		static constexpr platform::string_view llvm_mc_name { "llvm-mc" };
#endif
		static constexpr platform::string_view arch_hack_param { PSTR("@gas-arch=") };
		static constexpr platform::string_view default_output_name { PSTR("a.out") };
		static constexpr int wrapper_general_error_code         = 100;
		static constexpr int wrapper_llvm_mc_killed_error_code  = wrapper_general_error_code + 1;
		static constexpr int wrapper_llvm_mc_stopped_error_code = wrapper_general_error_code + 2;
		static constexpr int wrapper_fork_failed_error_code     = wrapper_general_error_code + 3;
		static constexpr int wrapper_exec_failed_error_code     = wrapper_general_error_code + 4;
		static constexpr int wrapper_wait_failed_error_code     = wrapper_general_error_code + 5;
	};

	enum class TargetArchitecture
	{
		Any,
		ARM32,
		ARM64,
		X86,
		X64,
	};
}
#endif
