// SPDX-License-Identifier: MIT
#if !defined (GAS_COMMAND_LINE_HH)
#define GAS_COMMAND_LINE_HH

#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <variant>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "constants.hh"

namespace xamarin::android::gas
{
#if !defined (_WIN32)
	using CommandLineString = std::string;
	using CommandLineStringView = std::string_view;
#else
	using CommandLineString = std::wstring;
	using CommandLineStringView = std::wstring_view;
#endif

	enum class Argument
	{
		Required,
		NotRequired,
	};

	struct CommandLineOption
	{
		CommandLineStringView name;
		uint32_t id;
		Argument argument;
		TargetArchitecture arch;

		constexpr CommandLineOption (CommandLineStringView const& _name, uint32_t _id, Argument _argument, TargetArchitecture _arch)
			: name (_name),
			  id (_id),
			  argument (_argument),
			  arch (_arch)
		{}

		constexpr CommandLineOption (CommandLineStringView const& _name, uint32_t _id, TargetArchitecture _arch)
			: CommandLineOption (_name, _id, Argument::NotRequired, _arch)
		{}

		constexpr CommandLineOption (CommandLineStringView const& _name, uint32_t _id, Argument _argument)
			: CommandLineOption (_name, _id, _argument, TargetArchitecture::Any)
		{}

		constexpr CommandLineOption (CommandLineStringView const& _name, uint32_t _id)
			: CommandLineOption (_name, _id, Argument::NotRequired, TargetArchitecture::Any)
		{}
	};

	class CommandLine
	{
#if !defined(_WIN32)
		static constexpr bool is_windows = false;
#else
		static constexpr bool is_windows = true;
#endif

	public:
#if !defined(_WIN32)
		using TArgType = char*;
		using TOptionString = std::string;

	private:
		static inline constexpr char DASH = '-';
		static inline constexpr char EQUALS = '=';
#else
		using TArgType = LPWSTR;
		using TOptionString = std::wstring;

	private:
		static inline constexpr wchar_t DASH = L'-';
		static inline constexpr wchar_t EQUALS = L'=';
#endif

	public:
		using TOptionValue = std::variant<bool, TOptionString>;
		using TCallbackOption = std::variant<uint32_t, const CommandLineOption>;
		using OptionCallbackFn = std::function<void(TCallbackOption option, TOptionValue val)>;

	public:
		explicit CommandLine (TargetArchitecture _target_arch) noexcept
			: target_arch (_target_arch)
		{}

		template<size_t NElem>
		bool parse (std::array<CommandLineOption, NElem> const& options, int argc, TArgType *argv, OptionCallbackFn option_cb)
		{
			return parse (std::span (options.data(), options.size()), argc, argv, option_cb);
		}

	private:
		bool parse (std::span<const CommandLineOption> options, int argc, TArgType *argv, OptionCallbackFn option_cb);

		static constexpr auto& stdout () noexcept
		{
			if constexpr (is_windows) {
				return std::wcout;
			} else {
				return std::cout;
			}
		}

		static constexpr auto& stderr () noexcept
		{
			if constexpr (is_windows) {
				return std::wcerr;
			} else {
				return std::cerr;
			}
		}

	private:
		TargetArchitecture target_arch;
	};
}

#if !defined (_WIN32)
#define CLISTR(_str_lit_) std::string_view { _str_lit_ }
#else
#define CLISTR(_str_lit_) std::wstring_view { L##_str_lit_ }
#endif

#endif // ndef GAS_COMMAND_LINE_HH
