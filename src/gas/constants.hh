// SPDX-License-Identifier: MIT
#if !defined (__CONSTANTS_HH)
#define __CONSTANTS_HH

#include "../shared/shared-constants.hh"

namespace xamarin::android::gas
{
	class Constants final
	{
	public:
#if defined (_WIN32)
		static constexpr char llvm_mc_name[] = "llvm-mc.exe";
#else
		static constexpr char llvm_mc_name[] = "llvm-mc";
#endif
		static constexpr char arch_hack_param[] = "@gas-arch=";
		static constexpr char default_output_name[] = "a.out";
	};
}
#endif
