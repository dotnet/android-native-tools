// SPDX-License-Identifier: MIT

#include <algorithm>
#include <iostream>
#include <optional>

#include "command_line.hh"

using namespace xamarin::android::gas;
namespace ranges = std::ranges;

bool CommandLine::parse (std::span<const CommandLineOption> options, std::vector<platform::string> &args, OptionCallbackFn option_cb)
{
	STDOUT << "Got " << options.size() << " options\n" << "Passed args:\n";

	uint32_t positional_count = 0;
	bool next_arg_is_value = false;
	std::optional<CommandLineOption> last_opt;

	for (size_t i = 1; i < args.size (); i++) {
		platform::string &option = args[i];
		if (option.empty ()) {
			continue;
		}

		STDOUT << "  [" << i << "] " << option << "\n";

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
				// TODO: throw, probably
			}

			next_arg_is_value = false;
			last_opt.reset ();
			continue;
		}

		bool positional = iter == option.cbegin ();
		STDOUT << std::boolalpha << "    Positional? " << positional << "\n";

		platform::string_view option_name;
		platform::string_view option_value;
		if (positional) {
			option_cb ({ positional_count++ }, { option });
			continue;
		}

		platform::string::const_iterator name_start = iter;
		while (iter != option.cend () && *iter != EQUALS) {
			iter++;
		}

		option_name = { name_start, iter };

		bool has_value = iter != option.cend ();
		STDOUT << "    Has value? " << has_value << "\n";

		if (has_value) {
			iter++;
			option_value = { iter, option.cend () };
		}

		STDOUT << "     Option name: " << platform::string (option_name) << "\n";
		STDOUT << "    Option value: " << platform::string (option_value) << "\n";

		auto matching_option = [this, &option_name] (CommandLineOption const& o) -> bool {
			if (o.name != option_name) {
				return false;
			}

			return o.arch == TargetArchitecture::Any || o.arch == target_arch;
		};

		auto match = ranges::find_if (options, matching_option);
		if (match != options.end ()) {
			STDOUT << "    found matching option\n";
		} else {
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
		STDERR << "Option '" << last_opt.value ().name << "' requires an argument" << std::endl;
		return false;
	}

	return true;
}
