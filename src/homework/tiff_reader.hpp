#pragma once

#include "core/gray_image.hpp"

#include <string>

namespace dip::homework {

[[nodiscard]] GrayImage read_uncompressed_tiff_gray_image(const std::string& path);

} // namespace dip::homework
