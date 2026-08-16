#include <mermaid_cpp.hpp>
#include <filesystem>

#define CANVAS_ITY_IMPLEMENTATION
#include <canvas_ity.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include <assert.h>


namespace fs = std::filesystem;

// default test app font
#ifndef MERMAID_TESTAPP_TTF_FILENAME
#define MERMAID_TESTAPP_TTF_FILENAME "/usr/share/fonts/TTF/DejaVuSans.ttf"
#endif

std::vector<char> load_ttf_file(const fs::path &p = MERMAID_TESTAPP_TTF_FILENAME) {
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

template <typename T, typename E>
constexpr decltype(auto) error_or(const T& t, const E& e) {
	return t.has_value() ? e : t.error();
}

bool render_diagram() {
	const size_t W = 100, H = 50;
	canvas_ity::canvas canvas(W, H);
	auto font = load_ttf_file(MERMAID_TESTAPP_TTF_FILENAME);
	if (!font.size()) {
		std::cerr << "Can't open ttf file\n";
		return false;
	}

	if (!canvas.set_font(reinterpret_cast<const unsigned char*>(font.data()), font.size(), 30)) {
		std::cerr << "Can't load ttf data\n";
	}
	canvas.set_color(canvas_ity::stroke_style, 1.0f, 0.9f, 0.2f, 1.0f );
	std::cerr << "test text width: " << canvas.measure_text("Test") << "\n" << std::endl;
	canvas.stroke_text("Test", 14, 35);

	std::vector<unsigned char> canvas_data(W*H*4);
	canvas.get_image_data(canvas_data.data(), W, H, W * 4, 0, 0);
	image_to_ascii(canvas_data, W, H, 4);
	image_data_to_file(canvas_data, W, H, 4, "out.png");

	return true;
}

bool test_app() {
	auto success_parse_status = mermaid_cpp::parse_api::parse_status::success();
	// empty test.
	{
		auto res_empty = mermaid_cpp::parse_api::parse_mermaid_md(R"(```mermaid
	```)");
		assert(!res_empty);
		std::cout << error_or(res_empty, success_parse_status) << std::endl;
	}

	// clang-format off
	auto result = mermaid_cpp::parse_api::parse_mermaid_md(R"(```mermaid
test
```)");
	// clang-format on
	std::cerr << "<expected> version: " <<  __cpp_lib_expected << std::endl;
	std::cerr << error_or(result, success_parse_status) << std::endl;
	assert(result);



	render_diagram();
	return result.operator bool();
}

bool test_invalid_app() {
	auto success_parse_status = mermaid_cpp::parse_api::parse_status::success();
	auto result = mermaid_cpp::parse_api::parse_mermaid_md(R"(```mermaid
vzhuh```)");
	std::cerr << "test invalid diag: " << error_or(result, success_parse_status) << std::endl;

	return false;
}

int main() {
	auto r1 = test_invalid_app();

	auto r = test_app();
	if (!r) {
		std::cerr << "test_app() failed\n";
	}
}
