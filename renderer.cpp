// SPDX-FileCopyrightText: 2026 mermaid-cpp contributors <https://github.com/mermaid-cpp/mermaid>
//
// SPDX-License-Identifier: MIT

#define CANVAS_ITY_IMPLEMENTATION
#include <canvas_ity.hpp>

#include <filesystem>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

// default renderer font
#ifndef MERMAID_TESTAPP_TTF_FILENAME
#define MERMAID_TESTAPP_TTF_FILENAME "/usr/share/fonts/TTF/DejaVuSans.ttf"
#endif

namespace fs = std::filesystem;

std::vector<char> load_ttf_file(const fs::path &p = MERMAID_TESTAPP_TTF_FILENAME);

void image_to_ascii(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels);

void image_data_to_file(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels, std::filesystem::path outfile);


bool render_diagram(std::string_view outfile) {
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
	image_data_to_file(canvas_data, W, H, 4, outfile);

	return true;
}


