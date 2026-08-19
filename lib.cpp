/**
 * @file        : lib.cpp
 * @created     : Thursday Sep 21, 2023 18:25:26 MSK
 */
#include <mermaid_cpp.hpp>

#include <concepts>
#include <expected>
#include <functional>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <string_view>

#include <assert.h>

#include <fmt/format.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

extern "C" int ff() {
#ifdef EMSCRIPTEN
	emscripten_run_script("alert('ff')");
#endif
	return 45;
}


namespace mermaid_cpp::parse_api {

class parse_state {
public:
	uint64_t line, column;
	std::string_view parsed_string;
	constexpr bool has_tokens() const noexcept {
		return parsed_string.size() > 3;
	}

	constexpr char cur_char() const noexcept {
		return parsed_string.front();
	}

	constexpr size_t increment_line() noexcept {
		column = 1;
		return ++line;
	}

	constexpr void advance_to_unsafe(size_t n) {
		column += n;
		parsed_string.remove_prefix(n);
	}
	constexpr bool advance_to(size_t n) {
		assert(parsed_string.size() >= n);
		if (parsed_string.size() < n) {
			return false;
		}
		advance_to_unsafe(n);
		return true;
	}

	template <typename T>
	constexpr bool advance_to_check(size_t n, T check_fn) {
		n = std::min(n, parsed_string.size());
		for (size_t i = 0; i < n; ++i) {
			if (check_fn(*this, parsed_string[0])) {
				parsed_string.remove_prefix(1);
			} else {
				return false;
			}
		}
		return true;
	}

	/// @brief Advances until check_fn returns true.
	/// @return true if check_fn() returns true.
	///         false if there are no symbols.
	template <typename T>
	constexpr bool advance_until(T check_fn) {
		return !advance_to_check(-1, std::not_fn(check_fn));
	}


	/// @brief Helper for creating parse_status from current parse state
	///        line/column.
	constexpr parse_status make_status(std::string_view msg, size_t range = 0, parse_status::type type = parse_status::type::error) const {
		return parse_status(line, column, range, std::string(msg), type);
	}

	constexpr std::unexpected<parse_status> make_unexpected_status(std::string_view msg, size_t range = 0, parse_status::type type = parse_status::type::error) const {
		return std::unexpected(make_status(msg, range, type));
	}
};

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

[[deprecated("Use starts_with_and_advance()")]] constexpr bool starts_with(const parse_state &st, std::string_view substr, bool count_chars = true) {
		return st.parsed_string.starts_with(substr);
}

// FIXME: extract all from starts_with
constexpr bool starts_with_and_advance(parse_state &st, std::string_view substr) {
	size_t size = std::min(substr.size(), st.parsed_string.size());
	scoped_state_restorer scope_restorer(st);

	for (auto i = 0; i < size; ++i) {
		if (substr[i] != st.parsed_string[i]) {
			return false;
		} else {
			st.column++;
		}

		// FIXME: for ease of impl we assume that \n increments line
		if (substr[i] == '\n') {
			st.increment_line();
		}
	}

	st.parsed_string.remove_prefix(size);
	scope_restorer.enable(false);
	return size == substr.size();
}

constexpr bool advance_check_newline_fn(parse_state &st, char c) noexcept {
	if (c == '\n') {
		st.increment_line();
		return true;
	}
	// sanity
	if (c == '\0') {
		return false;
	}
	st.column++;
	return true;
}

constexpr bool advance_to_next_newline_only_fn(parse_state &st, char c) noexcept {
	if (c == '\n') {
		st.increment_line();
		return false;
	}

	// sanity
	if (c == '\0') {
		return false;
	}

	st.column++;
	return true;
}

constexpr bool advance_to_n_check_newline(parse_state &st, size_t n) {
	return st.advance_to_check(n, advance_check_newline_fn);
}

constexpr bool advance_to_next_line(parse_state &st) {
	st.advance_to_check(-1, advance_to_next_newline_only_fn);
	if (!st.parsed_string.empty() && st.cur_char() == '\n') {
		st.advance_to(1);
		return true;
	}
	return false;
}

constexpr bool skip_all_ws(parse_state &st, bool skip_newline = true) {
	auto old_line = st.line;
	auto old_col = st.column;
	auto res = st.advance_to_check(st.parsed_string.size(),
		[&skip_newline](parse_state &st, char c) {
			if (c == ' ' || c == '\t' || c == '\r') {
				st.column++;
				return true;
			}
			if (c == '\n' && skip_newline) {
				return advance_check_newline_fn(st, c);
			}
			return false;
		});
	return st.line != old_line || st.column != old_col;
}

std::ostream &explain_parse_error(std::ostream &os, const mermaid_cpp::parse_api::parse_status &st, std::string_view context) {
	if (st) {
		return os;
	}

	std::string_view line_for_context = context;
	size_t counter = 1;
	while (counter < st.line()) {

		auto res = line_for_context.find_first_of("\n");
		if (res == std::string_view::npos) {
			break;
		}
		line_for_context.remove_prefix(res + 1);
		counter++;
	}
	if (auto res = line_for_context.find_first_of("\n"); res != std::string_view::npos) {
		line_for_context.remove_suffix(line_for_context.size() - res);
	}
	os << line_for_context << "\n";
	std::ios ios(nullptr);
	ios.copyfmt(os);
	os << std::setw(st.column()) << "^";
	if (st.range() != 0) {
		os << std::setfill('~') << std::setw(st.range() - 1) << "^\n";
	}
	os.copyfmt(ios);
	return os;
}

std::ostream & operator <<(std::ostream &os, const parse_status &status) {
	if (status == parse_status::success()) {
		os << "Success";
		return os;
	}

	static const std::string_view statuses[] = {
		"Success",
		"Info",
		"Warning",
		"Error",
	};
	os << statuses[(int)status.type_] << " at line " << status.line_ << ", column "
		<< status.column_;
	if (status.range_ > 1) {
		os << '-' << status.column_ + status.range_;
	}

	if (!status.message_.empty()) {
		os << ": " << status.message_;
	}
	return os;
}

// tests
static bool test_all() {
	constexpr parse_state state_c(1,1, "test_huest");
	parse_state state(1,1, "test_huest");
	static_assert(starts_with(state_c, "test_"));
	assert(starts_with_and_advance(state, "test_") && (state.column == 6));
	parse_state state2(1,1, "\n\n\nhuest");
	assert(skip_all_ws(state2) && state2.parsed_string == "huest" && state2.line == 4 && state2.column == 1);
	assert(!skip_all_ws(state2) && state2.parsed_string == "huest" && state2.line == 4 && state2.column == 1);

	parse_state state3(1,1, "test\nhuest\nvzhuh");
	assert(advance_to_next_line(state3) && state3.parsed_string == "huest\nvzhuh");
	assert(advance_to_next_line(state3) && state3.parsed_string == "vzhuh");
	assert(!advance_to_next_line(state3));
	return true;
}

static const bool test_it = test_all();

template <typename T = parse_state>
constexpr std::expected<std::unique_ptr<diagrams::diagram>, parse_status> parse_test_diagram(T &&state) {
	const auto &in = state.parsed_string;
	std::string_view test_label =[&in, &state]() {
		auto np = std::string_view::npos;
		auto newline_pos = in.find('\n');
		// no newline -> empty test diagram
		if (newline_pos == np) {
			return std::string_view("");
		}
		std::string_view new_label = in;
		new_label.remove_suffix(in.length() - newline_pos);
		// has markdown end marker -> empty test diagram
		if (new_label.find("```") != np) {
			return std::string_view("");
		}
		state.advance_to(newline_pos);
		return new_label;
	}();
	return std::make_unique<diagrams::test_diagram>(std::string{test_label.data(), test_label.size()});
}

static_assert(parse_test_diagram(parse_state{.parsed_string = "\nTest```"}).has_value());

constexpr std::expected<std::unique_ptr<diagrams::diagram>, parse_status> parse_mermaid(parse_state &state) {
	const auto &in = state.parsed_string;

	bool test_diag = false;
	if (starts_with_and_advance(state, "test")) {
		test_diag = true;
	}

	if (state.parsed_string.empty()) {
		return state.make_unexpected_status("Unexpected EOF while parsing diagram");
	}
	if (in[0] == '\n') {
		advance_to_n_check_newline(state, 1);
	} else {
		return state.make_unexpected_status(
			test_diag ? "Expected newline after diagram type" : "Diagram is empty", 1,
			test_diag ? parse_status::type::error : parse_status::type::info
		);
	}

	if (!test_diag) {
		return std::make_unique<diagrams::empty_diagram>();
	}
	return parse_test_diagram(state);
}

std::expected<std::unique_ptr<diagrams::diagram>, parse_status> parse_mermaid_md(std::string_view in) {
	parse_state state {1, 1, in};
	constexpr std::string_view mermaid_md_block_begin = "```mermaid";
	constexpr std::string_view mermaid_md_block_end = "```";

	if (!starts_with_and_advance(state, mermaid_md_block_begin)) {
		return state.make_unexpected_status("Expected markdown begin block token with mermaid block id(\"```mermaid\")", 3);
	}

	if (!state.has_tokens()) {
		return state.make_unexpected_status("Diagram is empty");
	}

	if (auto cur_chr = state.cur_char(); cur_chr != '\n') {
		return state.make_unexpected_status(
				fmt::format("Expected newline, got unknown token '{}'{}",
						cur_chr, cur_chr == '\0' ? "EOF" : ""), 1
				);
	} else {
		advance_to_n_check_newline(state, 1);
	}

	auto result = parse_mermaid(state);

	if (!result) {
		return result;
	}

	skip_all_ws(state);

	if (!starts_with(state, mermaid_md_block_end)) {
		return state.make_unexpected_status("Expected markdown end block token (\"```\")",3);
	}
	return result;
}

} // namespace mermaid_cpp::parse_api
