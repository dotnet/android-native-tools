// SPDX-License-Identifier: MIT
#if !defined (LLVM_STRIP_HH)
#define LLVM_STRIP_HH

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#if __has_include (<concepts>)
#include <concepts>
#endif // has <concepts>

#include "../shared/app.hh"

namespace cxxopts {
    class Options;
}

namespace xamarin::android::binutils {
	class Process;
}

namespace xamarin::android::llvm_strip {
	class LlvmStrip final : public binutils::App
	{
		static inline constexpr std::string_view PROGRAM_DESCRIPTION { "Xamarin.Android llvm-strip + GNU strip wrapper for llvm-objcopy" };

	public:
		virtual ~LlvmStrip () = default;

		int run (int argc, char **argv) override final;

	protected:
		binutils::ParseArgsResult parse_arguments (int argc, char **argv, binutils::Process &llvm_objcopy);

		std::string make_arg (std::string_view const& mark, std::string_view const& arg_name) const
		{
			std::string arg { mark };
			arg.append (arg_name);

			return arg;
		}

		std::string make_short_arg (std::string_view const& arg_name) const
		{
			constexpr std::string_view mark { "-" };
			return make_arg (mark, arg_name);
		}

		std::string make_long_arg (std::string_view const& arg_name) const
		{
			constexpr std::string_view mark { "--" };
			return make_arg (mark, arg_name);
		}

	private:
		cxxopts::Options create_options () override final;
	};
}
#endif // ndef LLVM_STRIP_HH
