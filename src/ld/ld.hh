// SPDX-License-Identifier: MIT
#if !defined (LD_HH)
#define LD_HH

#include <string_view>

#include "../shared/app.hh"

namespace cxxopts {
	class Options;
}

namespace xamarin::android::ld {
	class LD final : public binutils::App
	{
		static inline constexpr std::string_view PROGRAM_DESCRIPTION  { "Xamarin.Android ld wrapper for lld" };
		static inline constexpr std::string_view LLVM_LLD_NAME        { "lld" };
		static inline constexpr std::string_view LLVM_LLD_INVOKE_NAME { "ld" };

	public:
		virtual ~LD () = default;

		int run (int argc, char **argv) override final;

	private:
		cxxopts::Options create_options () override final;
	};
}
#endif // ndef LD_HH
