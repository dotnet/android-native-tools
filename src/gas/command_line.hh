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
#include "platform.hh"

namespace xamarin::android::gas
{
	enum class ArgumentValue
	{
		Required,
		NotRequired,
	};

	enum class OptionId : uint32_t
	{
		Ignore = 100,
		O,
		Warn,
		G,
		MFPU,
		Version,
		VersionExit,
		Help,
	};

	struct CommandLineOption
	{
		platform::string_view name;
		OptionId id;
		ArgumentValue argument;
		TargetArchitecture arch;

		constexpr CommandLineOption (platform::string_view const& _name, OptionId _id, ArgumentValue _argument, TargetArchitecture _arch)
			: name (_name),
			  id (_id),
			  argument (_argument),
			  arch (_arch)
		{}

		constexpr CommandLineOption (platform::string_view const& _name, OptionId _id, TargetArchitecture _arch)
			: CommandLineOption (_name, _id, ArgumentValue::NotRequired, _arch)
		{}

		constexpr CommandLineOption (platform::string_view const& _name, OptionId _id, ArgumentValue _argument)
			: CommandLineOption (_name, _id, _argument, TargetArchitecture::Any)
		{}

		constexpr CommandLineOption (platform::string_view const& _name, OptionId _id)
			: CommandLineOption (_name, _id, ArgumentValue::NotRequired, TargetArchitecture::Any)
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

	private:
		static inline constexpr char DASH = PCHAR('-');
		static inline constexpr char EQUALS = PCHAR('=');
#else
		using TArgType = LPWSTR;

	private:
		static inline constexpr wchar_t DASH = PCHAR('-');
		static inline constexpr wchar_t EQUALS = PCHAR('=');
#endif

	public:
		using TOptionValue = std::variant<bool, platform::string>;
		using TCallbackOption = std::variant<uint32_t, const CommandLineOption>;
		using OptionCallbackFn = std::function<void(TCallbackOption option, TOptionValue val)>;

	public:
		explicit CommandLine (TargetArchitecture _target_arch) noexcept
			: target_arch (_target_arch)
		{}

		template<size_t NElem>
		bool parse (std::array<CommandLineOption, NElem> const& options, std::vector<platform::string> &args, OptionCallbackFn option_cb)
		{
			return parse (std::span (options.data(), options.size()), args, option_cb);
		}

	private:
		bool parse (std::span<const CommandLineOption> options, std::vector<platform::string> &args, OptionCallbackFn option_cb);

	private:
		TargetArchitecture target_arch;
	};
}

#define CLISTR(_str_lit_) PSTR((_str_lit_))

#if !defined (_WIN32)
#define CLIPARAM(_str_lit_) std::string_view { _str_lit_ }
#else
#define CLIPARAM(_str_lit_) std::string_view { _str_lit_ }
#endif

#endif // ndef GAS_COMMAND_LINE_HH
