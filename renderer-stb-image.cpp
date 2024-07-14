#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>
#include <filesystem>
#include <span>

// Function to convert image
void image_data_to_file(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels, std::filesystem::path outfile) {
	stbi_write_png(outfile.generic_string().c_str(), width, height, 4, data.data(), width * channels);
}
