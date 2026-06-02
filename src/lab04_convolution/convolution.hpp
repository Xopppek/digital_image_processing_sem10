#pragma once

#include "core/gray_image.hpp"

#include <cstddef>
#include <vector>

namespace dip::lab04 {

struct Kernel2D {
    std::size_t width{0};
    std::size_t height{0};
    std::vector<double> values;
};

struct ConvolutionResponse {
    std::size_t width{0};
    std::size_t height{0};
    std::vector<double> values;
};

[[nodiscard]] ConvolutionResponse convolve_valid_response(const GrayImage& image, const Kernel2D& kernel);
[[nodiscard]] GrayImage convolve_valid(const GrayImage& image, const Kernel2D& kernel);

} // namespace dip::lab04
