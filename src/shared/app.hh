// SPDX-License-Identifier: MIT
#if !defined (XABINUTILS_SHARED_APP_HH)
#define XABINUTILS_SHARED_APP_HH

#include <filesystem>
#include <string>

namespace cxxopts {
	class Options;
}

namespace xamarin::android::binutils {
	struct ParseArgsResult
	{
		const bool terminate;
		const bool is_error;

		ParseArgsResult (bool _terminate, bool _is_error) noexcept
			: terminate (_terminate),
			  is_error (_is_error)
		{}
	};

	class App
	{
	public:
		virtual ~App () = default;

		virtual int run (int argc, char **argv) = 0;

		void get_command_line (int &argc, char **&argv);
		const std::string& program_name () const noexcept
		{
			return _program_name;
		}

		void set_program_name (std::string new_name)
		{
			_program_name = new_name;
		}

		const std::filesystem::path& program_dir () const noexcept
		{
			return _program_dir;
		}

	protected:
		void determine_program_dir (int argc, char **argv);
		int usage (bool is_error, std::string const message = "");
		virtual cxxopts::Options create_options () = 0;

		virtual void usage_print_extra ([[maybe_unused]] bool is_error)
		{}

	private:
		std::string           _program_name;
		std::filesystem::path _program_dir;
	};
}
#endif // ndef XABINUTILS_SHARED_APP_HH
