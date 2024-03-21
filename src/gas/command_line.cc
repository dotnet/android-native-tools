// SPDX-License-Identifier: MIT

#include <algorithm>
#include <iostream>

#include "command_line.hh"

using namespace xamarin::android::gas;
namespace ranges = std::ranges;

bool CommandLine::parse (std::span<const CommandLineOption> options, int argc, TArgType *argv, OptionCallbackFn option_cb)
{
	stdout() << "Got " << options.size() << " options\n" << "Passed args:\n";

	uint32_t positional_count = 0;
	bool next_must_be_positional = false;

	for (int i = 1; i < argc; i++) {
		TOptionString option { argv[i] };
		if (option.empty ()) {
			continue;
		}

		stdout() << "  [" << i << "] " << option << "\n";

		TOptionString::const_iterator iter = option.cbegin ();

		while (iter != option.cend () && *iter == DASH) {
			iter++;
		}
		bool positional = iter == option.cbegin ();
		stdout() << std::boolalpha << "    Positional? " << positional << "\n";

		CommandLineStringView option_name;
		CommandLineStringView option_value;
		if (positional) {
			option_cb ({ positional_count++ }, { option });
			continue;
		}

		TOptionString::const_iterator name_start = iter;

		while (iter != option.cend () && *iter != EQUALS) {
			iter++;
		}

		option_name = { name_start, iter };

		bool has_value = iter != option.cend ();
		stdout() << "    Has value? " << has_value << "\n";

		if (has_value) {
			iter++;
			option_value = { iter, option.cend () };
		}

		stdout() << "     Option name: " << TOptionString (option_name) << "\n";
		stdout() << "    Option value: " << TOptionString (option_value) << "\n";

		auto matching_option = [this, &option_name] (CommandLineOption const& o) -> bool {
			if (o.name != option_name) {
				return false;
			}

			return o.arch == TargetArchitecture::Any || o.arch == target_arch;
		};

		auto match = ranges::find_if (options, matching_option);
		if (match != options.end ()) {
			stdout() << "    found matching option\n";
		} else {
			stderr() << "Uncrecognized option '" << option << "'\n";
		}
	}

	return true;
}
