/// @file renderer-fmt-console.cpp Contains simple renderer based on some simplest
/// thoughts from Conan-Center's blog.
#include <fmt/core.h>
#include <fmt/color.h>

#include <stddef.h>

#include <span>
#include <string_view>
// https://blog.conan.io/2024/03/21/Introducing-new-conan-visual-studio-extension.html
// Size of ASCII art
static const size_t new_width = 80;

// Ascii gradient
static const std::string_view ASCII_CHARS = " .:-=+#%@@@";

// Function to scale the luminance into an ASCII character
char map_luminance_to_ascii(float luminance) {
    size_t position = luminance * (ASCII_CHARS.size() - 1);
    return ASCII_CHARS[position];
}

// Function to convert image to ASCII art
void image_to_ascii(const std::span<unsigned char> &data, size_t width, size_t height, size_t channels) {
    // Adjust aspect ratio for ASCII art
    size_t new_height = static_cast<int>(static_cast<double>(height) / width * new_width * 0.45);

    for (size_t i = 0; i < new_height; ++i) {
        for (size_t j = 0; j < new_width; ++j) {
            size_t old_i = i * height / new_height;
            size_t old_j = j * width / new_width;

            float r = data[(old_i * width + old_j) * channels + 0] / 255.0f;
            float g = data[(old_i * width + old_j) * channels + 1] / 255.0f;
            float b = data[(old_i * width + old_j) * channels + 2] / 255.0f;
            float luminance = (0.2126f * r + 0.7152f * g + 0.0722f * b);

            char ascii_char = map_luminance_to_ascii(luminance);

            // Use fmt to print ASCII character with color
            fmt::print(fmt::fg(fmt::rgb(uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255))), "{}", ascii_char);
        }
        fmt::print("\n");
    }
}

