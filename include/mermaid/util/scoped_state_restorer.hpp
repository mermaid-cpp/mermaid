// SPDX-FileCopyrightText: 2026 mermaid-cpp contributors <https://github.com/mermaid-cpp/mermaid>
//
// SPDX-License-Identifier: MIT

#ifndef LB_MERMAID_UTIL_HPP_
#define LB_MERMAID_UTIL_HPP_
#include <ranges>

namespace mermaid::util {
template <typename T>
requires std::copyable<T>
class scoped_state_restorer {
	T prev_state;
	T &state_to_restore;
	bool enabled_;
public:
	constexpr scoped_state_restorer(T &state, bool enabled = true)
		: prev_state(state), state_to_restore(state), enabled_(enabled) {}
	constexpr bool is_enabled() const noexcept {
		return enabled_;
	}

	constexpr void enable(bool new_enabled) noexcept {
		enabled_ = new_enabled;
	}

	constexpr ~scoped_state_restorer() {
		if (enabled_) {
			state_to_restore = prev_state;
		}
	}
};

template <typename T>
scoped_state_restorer(T &, bool) -> scoped_state_restorer<T>;


} // namespace mermaid::util

#endif // LB_MERMAID_UTIL_HPP_
