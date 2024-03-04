// SPDX-License-Identifier: MIT
#if !defined (XABINUTILS_SHARED_CONSTANTS_HH)
#define XABINUTILS_SHARED_CONSTANTS_HH

namespace xamarin::android::binutils {
	class SharedConstants final
	{
	public:
#if defined (_WIN32)
		static constexpr char newline[] = "\r\n";
#else
		static constexpr char newline[] = "\n";
#endif
		static constexpr int wrapper_general_error_code         = 100;
		static constexpr int wrapper_fork_failed_error_code     = wrapper_general_error_code + 1;
		static constexpr int wrapper_exec_failed_error_code     = wrapper_general_error_code + 2;
		static constexpr int wrapper_wait_failed_error_code     = wrapper_general_error_code + 3;
		static constexpr int wrapper_process_killed_error_code  = wrapper_general_error_code + 4;
		static constexpr int wrapper_process_stopped_error_code = wrapper_general_error_code + 5;

		static constexpr int wrapper_custom_error_code_base     = wrapper_general_error_code + 100;
	};
}

#endif // ndef XABINUTILS_SHARED_CONSTANTS_HH
