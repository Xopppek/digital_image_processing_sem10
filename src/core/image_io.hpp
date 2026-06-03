#pragma once

#include "core/gray_image.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dip {

struct RgbPixel {
    std::uint8_t red{0};
    std::uint8_t green{0};
    std::uint8_t blue{0};
};

struct RgbImage {
    std::size_t width{0};
    std::size_t height{0};
    std::vector<RgbPixel> pixels;
};

GrayImage read_gray_image(const std::string& path);
void write_gray_image(const std::string& path, const GrayImage& image);
void write_rgb_image(const std::string& path, const RgbImage& image);

} // namespace dip
