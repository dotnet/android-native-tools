// SPDX-License-Identifier: MIT
#if !defined (__PROCESS_HH)
#define __PROCESS_HH

#include <filesystem>
#include <string>
#include <variant>
#include <vector>
#include "gas.hh"

namespace xamarin::android::gas
{
	class Process
	{
	public:
		using string_list = std::vector<std::string>;
		using process_argument = std::variant<std::string, string_list>;

	public:
		explicit Process (fs::path const& executable_path)
			: executable_path (executable_path.lexically_normal ())
		{}

		int run (bool print_command_line = true);
		void append_program_argument (std::string const& option_name, std::string const& option_value = "");
		void append_program_argument (std::string const& option_name, string_list const& option_value, bool uses_comma_separated_list = false);

		void append_program_argument (std::string const& option_name, process_argument const& option_value, bool uses_comma_separated_list = false)
		{
			switch (option_value.index ()) {
				case 0:
					append_program_argument (option_name, std::get<std::string> (option_value));
					break;

				case 1:
					append_program_argument (option_name, std::get<string_list> (option_value), uses_comma_separated_list);
					break;
			}
		}

		std::vector<std::string> const& args () const noexcept
		{
			return _args;
		}

	private:
		void print_process_command_line ();
		std::vector<std::string::const_pointer> make_exec_args ();

	private:
		std::vector<std::string> _args;
		fs::path const executable_path;
	};
}
#endif
