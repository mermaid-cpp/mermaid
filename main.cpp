#include <mermaid/mermaid.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include <assert.h>


namespace fs = std::filesystem;

std::vector<char> load_ttf_file(const fs::path &p) {
	std::ifstream font(p.generic_string().c_str(), std::ios::binary);
	if (!font) {
		return {};
	}
	auto sz = fs::file_size(p);
	std::vector<char> vec(sz);
	font.read(vec.data(), sz);
	return vec;
}

void image_to_ascii(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels);

void image_data_to_file(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels, std::filesystem::path outfile);

bool render_diagram(std::string_view outfile = "out.png")
#ifdef MERMAID_NO_RENDERERS
{
	std::cerr << "Renderers are disabled.\n";
	return false;
}
#endif
;

bool test_app() {
	auto success_parse_status = mermaid::syntax::parse_status::success();
	// empty test.
	{
		auto res_empty = mermaid::syntax::parse_mermaid_md(R"(```mermaid
	```)");
		assert(!res_empty);
		std::cout << res_empty.error_or(success_parse_status) << std::endl;
	}

	// clang-format off
	auto result = mermaid::syntax::parse_mermaid_md(R"(```mermaid
test
```)");
	// clang-format on
	std::cerr << result.error_or(success_parse_status) << std::endl;
	assert(result);

	render_diagram();
	return result.has_value();
}

bool test_invalid_app() {
	auto success_parse_status = mermaid::syntax::parse_status::success();
	auto result = mermaid::syntax::parse_mermaid_md(R"(```mermaid
vzhuh```)");
	std::cerr << "test invalid diag: " << result.error_or(success_parse_status) << std::endl;

	return false;
}

int main() {
	auto r1 = test_invalid_app();

	auto r = test_app();
	if (!r) {
		std::cerr << "test_app() failed\n";
	}
}
