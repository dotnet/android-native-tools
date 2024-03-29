// SPDX-License-Identifier: MIT

#include <algorithm>
#include <optional>
#include <vector>

#include "command_line.hh"
#include "platform.hh"

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
				STDERR << "Option '" << option << "' requires an argument.";
				return false;
			}

			next_arg_is_value = false;
			last_opt.reset ();
			continue;
		}

		if (iter == option.cbegin ()) { // positional
			option_cb ({ positional_count++ }, { option });
			continue;
		}

		platform::string::const_iterator name_start = iter;
		while (iter != option.cend () && *iter != EQUALS) {
			iter++;
		}

#if !defined (__APPLE__)
		platform::string_view option_name { name_start, iter };
#else
		platform::string_view option_name (option.data () + (name_start - option.cbegin ()), iter - name_start);
#endif
		platform::string_view option_value;

		if (iter != option.cend ()) { // has a value
			iter++;
#if !defined (__APPLE__)
			option_value = { iter, option.cend () };
#else
			option_value = platform::string_view (option.data () + (iter - option.cbegin ()), option.cend () - iter);
#endif
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
		auto match = std::find_if (options.begin (), options.end (), matching_option);
#endif
		if (match == options.end ()) {
			STDERR << "Unrecognized option '" << option << Constants::newline;
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
		STDERR << "Option '" << last_opt.value().name << "' requires an argument." << std::endl;
		return false;
	}

	return true;
}
