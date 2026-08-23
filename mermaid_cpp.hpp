#ifndef LB_MERMAID_CPP_H_
#define LB_MERMAID_CPP_H_
#include <expected>
#include <iosfwd>
#include <memory>
#include <string_view>
#include <string>

namespace mermaid_cpp::diagrams {

class diagram {
public:
	virtual ~diagram() = default;
	virtual std::string_view name() const noexcept = 0;
};

class empty_diagram : public diagram {
public:
	std::string_view name() const noexcept override {
		return "";
	}
};

class test_diagram : public diagram {
public:
	constexpr test_diagram(std::string l = ""): label(l) {}
	std::string_view name() const noexcept override {
		return "test";
	}
	constexpr bool operator ==(const test_diagram &d) const noexcept {
		return label == d.label;
	}
private:
	std::string label = "";
};

} // namespace mermaid_cpp::diagrams

namespace mermaid_cpp::parse_api {

class parse_status {
public:
	enum class type {
		success,
		info,
		warning,
		error,
	};
private:
	uint64_t line_ = 0, column_ = 0, range_ = 0;
	type type_ = type::success;
	std::string message_;
public:

	constexpr parse_status(uint64_t line = 0, uint64_t column = 0,
			uint64_t range = 0, std::string &&msg = std::string(""), type status_type = type::error)
	: line_(line), column_(column), range_(range), type_(status_type), message_(std::move(msg))
	{}

	constexpr auto line() const noexcept {
		return line_;
	}

	constexpr auto column() const noexcept {
		return column_;
	}

	constexpr auto range() const noexcept {
		return range_;
	}

	constexpr bool operator ==(const parse_status &st) const noexcept {
		return line_ == st.line_ && column_ == st.column_ && range_ == st.range_ && type_ == st.type_;
	}

	constexpr bool operator !=(const parse_status &st) const noexcept {
		return !((*this) == st);
	}

	constexpr operator bool() const noexcept {
		return (*this) == parse_status::success();
	}

	static constexpr parse_status success() noexcept {
		return {};
	}

	static constexpr parse_status info(std::string_view msg, size_t line, size_t column, size_t range) noexcept {
		return {line, column, range, std::string(msg), type::info};
	}

	friend std::ostream & operator <<(std::ostream &os, const parse_status &status);
};


std::ostream & operator <<(std::ostream &os, const parse_status &status);

std::expected<std::unique_ptr<diagrams::diagram>, parse_status> parse_mermaid_md(std::string_view in);

std::ostream &explain_parse_error(std::ostream &os, const mermaid_cpp::parse_api::parse_status &st, std::string_view context);

} // namespace mermaid::parse_api

#endif // LB_MERMAID_CPP_H_
