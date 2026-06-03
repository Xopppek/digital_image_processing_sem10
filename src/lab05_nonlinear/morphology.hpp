#pragma once

#include "core/gray_image.hpp"
#include "lab05_nonlinear/rank_filter.hpp"

namespace dip::lab05 {

[[nodiscard]] GrayImage erosion_valid(const GrayImage& image, const Aperture& structuring_element);
[[nodiscard]] GrayImage dilation_valid(const GrayImage& image, const Aperture& structuring_element);
[[nodiscard]] GrayImage opening_valid(const GrayImage& image, const Aperture& structuring_element);
[[nodiscard]] GrayImage closing_valid(const GrayImage& image, const Aperture& structuring_element);

} // namespace dip::lab05
