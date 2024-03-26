// SPDX-License-Identifier: MIT

#include <algorithm>
#include <iostream>
#include <optional>

#include "command_line.hh"
#include "exceptions.hh"

using namespace xamarin::android::gas;
namespace ranges = std::ranges;

bool CommandLine::parse (std::span<const CommandLineOption> options, std::vector<platform::string> &args, OptionCallbackFn option_cb)
{
	uint32_t positional_count = 0;
	bool next_arg_is_value = false;
	std::optional<CommandLineOption> last_opt;

	for (size_t i = 1; i < args.size (); i++) {
		platform::string &option = args[i];
		if (option.empty ()) {
			continue;
		}

		platform::string::const_iterator iter = option.cbegin ();
		while (iter != option.cend () && *iter == DASH) {
			iter++;
		}

		if (next_arg_is_value) {
			// getopt takes the next argument verbatim, if separated from the option requiring a value by a space.
			// This is done regardless of whether or not the next argument is prefixed with `-` or is a known one.
			if (last_opt.has_value ()) {
				option_cb (last_opt.value (), option);
			} else {
				throw invalid_argument_error { "Option '" + option + "' requires an argument." };
			}

			next_arg_is_value = false;
			last_opt.reset ();
			continue;
		}

		platform::string_view option_name;
		platform::string_view option_value;
		if (iter == option.cbegin ()) { // positional
			option_cb ({ positional_count++ }, { option });
			continue;
		}

		platform::string::const_iterator name_start = iter;
		while (iter != option.cend () && *iter != EQUALS) {
			iter++;
		}

		option_name = { name_start, iter };

		if (iter != option.cend ()) { // has a value
			iter++;
			option_value = { iter, option.cend () };
		}

		auto matching_option = [this, &option_name] (CommandLineOption const& o) -> bool {
			if (o.name != option_name) {
				return false;
			}

			return o.arch == TargetArchitecture::Any || o.arch == target_arch;
		};

#if !defined(__APPLE__)
		auto match = ranges::find_if (options, matching_option);
#else
		// C++ standard library on mac CI doesn't have std::ranges::find_if
		auto match = std::find_if (options. matching_option);
#endif
		if (match == options.end ()) {
			STDERR << "Uncrecognized option '" << option << "'\n";
			continue;
		}

		CommandLineOption opt = *match;
		if (opt.argument == ArgumentValue::Required && option_value.empty ()) {
			next_arg_is_value = true;
			last_opt = opt;
			continue;
		}

		option_cb (opt, platform::string {option_value});
	}

	if (last_opt.has_value ()) {
		platform::string message { "Option '" };
		message
			.append (last_opt.value ().name)
			.append ("' requires an argument.");
		throw invalid_operation_error {message};
	}

	return true;
}
