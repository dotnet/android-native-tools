// SPDX-License-Identifier: MIT
#if !defined (__CONSTANTS_HH)
#define __CONSTANTS_HH

namespace xamarin::android::gas
{
	class Constants final
	{
	public:
#if defined (_WIN32)
		static constexpr char newline[] = "\r\n";
		static constexpr char llvm_mc_name[] = "llvm-mc.exe";
#else
		static constexpr char newline[] = "\n";
		static constexpr char llvm_mc_name[] = "llvm-mc";
#endif
		static constexpr char arch_hack_param[] = "@gas-arch=";
		static constexpr char default_output_name[] = "a.out";
		static constexpr int wrapper_general_error_code         = 100;
		static constexpr int wrapper_llvm_mc_killed_error_code  = wrapper_general_error_code + 1;
		static constexpr int wrapper_llvm_mc_stopped_error_code = wrapper_general_error_code + 2;
		static constexpr int wrapper_fork_failed_error_code     = wrapper_general_error_code + 3;
		static constexpr int wrapper_exec_failed_error_code     = wrapper_general_error_code + 4;
		static constexpr int wrapper_wait_failed_error_code     = wrapper_general_error_code + 5;
	};
}
#endif
