// SPDX-License-Identifier: MIT
#if !defined (XABINUTILS_SCOPE_GUARD_HH)
#define XABINUTILS_SCOPE_GUARD_HH

#include <utility>

#if __has_include (<concepts>)
#include <concepts>
#endif // has <concepts>

namespace xamarin::android::binutils {
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
}
#endif // ndef XABINUTILS_SCOPE_GUARD_HH
