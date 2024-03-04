// SPDX-License-Identifier: MIT
#if !defined (__GAS_HH)
#define __GAS_HH

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "../shared/app.hh"

namespace cxxopts {
	class Options;
}

namespace xamarin::android::gas
{
	namespace fs = std::filesystem;

	class LlvmMcRunner;

	enum class TargetArchitecture
	{
		ARM32,
		ARM64,
		X86,
		X64,
	};

	enum OptionValues
	{
		OPTION_IGNORE = 100,
		OPTION_O,
		OPTION_WARN,
		OPTION_G,
		OPTION_MFPU,
		OPTION_VERSION,
		OPTION_VERSION_EXIT,
		OPTION_HELP,
	};

	template<size_t Size>
	struct helper_char_array final
	{
		constexpr char* data () noexcept
		{
			return _elems;
		}

		constexpr const char* data () const noexcept
		{
			return _elems;
		}

		constexpr char const& operator[] (size_t n) const noexcept
		{
			return _elems[n];
		}

		constexpr char& operator[] (size_t n) noexcept
		{
			return _elems[n];
		}

		char _elems[Size]{};
	};

	// MinGW 9 on the CI build bots has a bug in the gcc compiler which causes builds to fail with:
	//
	//  error G713F753E: ‘constexpr auto xamarin::android::concat_const(const char (&)[Length]...) [with long long unsigned int ...Length = {15, 7, 5}]’ called in a constant expression
	//  ...
	//  /usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/include/c++/array:94:12: note: ‘struct std::array<char, 17>’ has no user-provided default constructor
	// struct array
	// ^~~~~
	// /usr/lib/gcc/x86_64-w64-mingw32/9.3-win32/include/c++/array:110:56: note: and the implicitly-defined constructor does not initialize ‘char std::array<char, 17>::_M_elems [17]’
	//  typename _AT_Type::_Type                         _M_elems;
	//                                                   ^~~~~~~~
	//
	// thus we need to use this workaround here
	//
#if defined (__MINGW32__) && __GNUC__ < 10
	template<size_t Size>
	using char_array = helper_char_array<Size>;
#else
	template<size_t Size>
	using char_array = std::array<char, Size>;
#endif

	template<size_t ...Length>
	static constexpr auto concat_const (const char (&...parts)[Length])
	{
		// `parts` being constant string arrays, Length for each of them includes the trailing NUL byte, thus the
		// `sizeof... (Length)` part which subtracts the number of template parameters - the amount of NUL bytes so that
		// we don't waste space.
		constexpr size_t total_length = (... + Length) - sizeof... (Length);
		char_array<total_length + 1> ret;
		ret[total_length] = 0;

		size_t i = 0;
		for (char const* from : {parts...}) {
			for (; *from != '\0'; i++) {
				ret[i] = *from++;
			}
		}

		return ret;
	}

	class Gas final : public binutils::App
	{
		static inline constexpr std::string_view PROGRAM_DESCRIPTION { "Xamarin.Android GAS adapter for llvm-mc" };

		static constexpr char generic_gas_name[] = "as";
		static constexpr char generic_ld_name[] =
#if defined (_WIN32)
			"ld.exe"
#else
			"ld"
#endif
			;

		static constexpr char arm64_arch_prefix[] = "aarch64-linux-android-";
		static constexpr char arm32_arch_prefix[] = "arm-linux-androideabi-";
		static constexpr char x86_arch_prefix[] = "i686-linux-android-";
		static constexpr char x64_arch_prefix[] = "x86_64-linux-android-";

	public:
		virtual ~Gas () = default;

		int run (int argc, char **argv) override final;

		TargetArchitecture target_arch () const noexcept
		{
			return _target_arch;
		}

	protected:
		binutils::ParseArgsResult parse_arguments (int argc, char **argv, std::unique_ptr<LlvmMcRunner>& mc_runner);
		void usage_print_extra (bool is_error) override final;
		cxxopts::Options create_options () override final;

	private:
		static constexpr auto arm64_gas_name = concat_const (arm64_arch_prefix, generic_gas_name);
		static constexpr auto arm32_gas_name = concat_const (arm32_arch_prefix, generic_gas_name);
		static constexpr auto x86_gas_name   = concat_const (x86_arch_prefix, generic_gas_name);
		static constexpr auto x64_gas_name   = concat_const (x64_arch_prefix, generic_gas_name);

		static constexpr auto arm64_ld_name  = concat_const (arm64_arch_prefix, generic_ld_name);
		static constexpr auto arm32_ld_name  = concat_const (arm32_arch_prefix, generic_ld_name);
		static constexpr auto x86_ld_name    = concat_const (x86_arch_prefix, generic_ld_name);
		static constexpr auto x64_ld_name    = concat_const (x64_arch_prefix, generic_ld_name);

		std::vector<fs::path> input_files;
		fs::path            _gas_output_file;
		TargetArchitecture  _target_arch;
	};
}

extern xamarin::android::gas::Gas app;
#endif // __GAS_HH
