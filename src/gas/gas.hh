// SPDX-License-Identifier: MIT
#if !defined (__GAS_HH)
#define __GAS_HH

#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <getopt.h>

#include <array>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "command_line.hh"
#include "constants.hh"
#include "platform.hh"

namespace xamarin::android::gas
{
	namespace fs = std::filesystem;

	template<class TFunc>
#if __has_include (<concepts>) && !defined(__APPLE__) // Apple clang reports it supports concepts, but it breaks on the next line
	requires std::invocable<TFunc>
#endif // has <concepts>
	class ScopeGuard
	{
	public:
		explicit ScopeGuard (TFunc&& fn) noexcept
			: fn (std::forward<TFunc> (fn))
		{}

		ScopeGuard (ScopeGuard const&) = delete;

		~ScopeGuard ()
		{
			fn ();
		}

		ScopeGuard operator= (ScopeGuard const&) = delete;
		ScopeGuard operator= (ScopeGuard &&) = delete;

	private:
		TFunc fn;
	};

	class LlvmMcRunner;

	template<class T>
	concept StringViewPart = std::is_same_v<T, platform::string_view>;

	template<size_t TotalLength, StringViewPart ...T>
	consteval auto concat_string_views (T const&... parts)
	{
		std::array<platform::string_view::value_type, TotalLength + 1> ret; // lgtm [cpp/paddingbyteinformationdisclosure] the buffer is filled in the loop below
		ret[TotalLength] = 0;

		size_t i = 0;
		for (auto const& sv : {parts...}) {
			for (platform::string_view::value_type ch : sv) {
				ret[i] = ch;
				i++;
			}
		}

		return ret;
	}

	consteval size_t calc_size (platform::string_view const& sv1) noexcept
	{
		return sv1.size ();
	}

	template<StringViewPart ...T>
	consteval size_t calc_size (platform::string_view const& sv1, T const&... other_svs) noexcept
	{
		return sv1.size () + calc_size (other_svs...);
	}

	class Gas final
	{
		struct ParseArgsResult
		{
			const bool terminate;
			const bool is_error;

			ParseArgsResult (bool _terminate, bool _is_error) noexcept
				: terminate (_terminate),
				  is_error (_is_error)
			{}
		};

		static constexpr platform::string_view generic_gas_name { PSTR("as") };
		static constexpr platform::string_view generic_ld_name {
#if defined (_WIN32)
			PSTR("ld.exe")
#else
			PSTR("ld")
#endif
		};

		static constexpr platform::string_view arm64_arch_prefix { PSTR("aarch64-linux-android-") };
		static constexpr platform::string_view arm32_arch_prefix { PSTR("arm-linux-androideabi-") };
		static constexpr platform::string_view x86_arch_prefix { PSTR("i686-linux-android-") };
		static constexpr platform::string_view x64_arch_prefix { PSTR("x86_64-linux-android-") };

	public:
		~Gas ()
		{}

		std::vector<platform::string> get_command_line (int &argc, char **&argv);

		int run (std::vector<platform::string> args);

		const platform::string& program_name () const noexcept
		{
			return _program_name;
		}

		const fs::path& program_dir () const noexcept
		{
			return _program_dir;
		}

		TargetArchitecture target_arch () const noexcept
		{
			return _target_arch;
		}

	protected:
		ParseArgsResult parse_arguments (std::vector<platform::string> &args, std::unique_ptr<LlvmMcRunner>& mc_runner);

	private:
		void determine_program_dir (std::vector<platform::string> args);
		int usage (bool is_error, platform::string const message = PSTR(""));

	private:
		static constexpr size_t arm64_gas_name_size = calc_size (arm64_arch_prefix, generic_gas_name);
		static constexpr auto arm64_gas_name        = concat_string_views<arm64_gas_name_size> (arm64_arch_prefix, generic_gas_name);

		static constexpr size_t arm32_gas_name_size = calc_size (arm32_arch_prefix, generic_gas_name);
		static constexpr auto arm32_gas_name        = concat_string_views<arm32_gas_name_size> (arm32_arch_prefix, generic_gas_name);

		static constexpr size_t x86_gas_name_size   = calc_size (x86_arch_prefix, generic_gas_name);
		static constexpr auto x86_gas_name          = concat_string_views<x86_gas_name_size> (x86_arch_prefix, generic_gas_name);

		static constexpr size_t x64_gas_name_size   = calc_size (x64_arch_prefix, generic_gas_name);
		static constexpr auto x64_gas_name          = concat_string_views<x64_gas_name_size> (x64_arch_prefix, generic_gas_name);

		static constexpr size_t arm64_ld_name_size  = calc_size (arm64_arch_prefix, generic_ld_name);
		static constexpr auto arm64_ld_name         = concat_string_views<arm64_ld_name_size> (arm64_arch_prefix, generic_ld_name);

		static constexpr size_t arm32_ld_name_size  = calc_size (arm32_arch_prefix, generic_ld_name);
		static constexpr auto arm32_ld_name         = concat_string_views<arm32_ld_name_size> (arm32_arch_prefix, generic_ld_name);

		static constexpr size_t x86_ld_name_size    = calc_size (x86_arch_prefix, generic_ld_name);
		static constexpr auto x86_ld_name           = concat_string_views<x86_ld_name_size> (x86_arch_prefix, generic_ld_name);

		static constexpr size_t x64_ld_name_size    = calc_size (x64_arch_prefix, generic_ld_name);
		static constexpr auto x64_ld_name           = concat_string_views<x64_ld_name_size> (x64_arch_prefix, generic_ld_name);

		std::vector<fs::path> input_files;

		platform::string    _program_name;
		fs::path            _gas_output_file;
		fs::path            _program_dir;
		TargetArchitecture  _target_arch;
	};
}

#endif // __GAS_HH
