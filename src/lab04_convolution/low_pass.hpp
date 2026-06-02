#pragma once

#include "core/gray_image.hpp"
#include "lab04_convolution/convolution.hpp"

#include <cstddef>

namespace dip::lab04 {

[[nodiscard]] Kernel2D make_average_low_pass_kernel(std::size_t size);
[[nodiscard]] GrayImage average_low_pass_filter(const GrayImage& image, std::size_t kernel_size);
[[nodiscard]] GrayImage thresholded_average_filter(
    const GrayImage& image,
    std::size_t kernel_size,
    double threshold
);

} // namespace dip::lab04
