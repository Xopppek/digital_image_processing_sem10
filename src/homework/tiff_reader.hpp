#pragma once

#include "core/gray_image.hpp"
#include "core/image_io.hpp"

#include <string>

namespace dip::homework {

[[nodiscard]] GrayImage read_uncompressed_tiff_gray_image(const std::string& path);
[[nodiscard]] RgbImage read_uncompressed_tiff_rgb_image(const std::string& path);

} // namespace dip::homework
