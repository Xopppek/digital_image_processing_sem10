#pragma once

#include "core/gray_image.hpp"

#include <string>

namespace dip {

GrayImage read_gray_image(const std::string& path);
void write_gray_image(const std::string& path, const GrayImage& image);

} // namespace dip
