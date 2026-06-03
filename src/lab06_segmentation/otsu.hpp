#pragma once

#include "core/gray_image.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace dip::lab06 {

struct OtsuResult {
    std::uint8_t threshold{0};
    std::array<std::size_t, 2> class_pixel_counts{0, 0};
    std::array<double, 2> class_probabilities{0.0, 0.0};
    std::array<double, 2> class_means{0.0, 0.0};
    double within_class_variance{0.0};
    double between_class_variance{0.0};
};

[[nodiscard]] OtsuResult otsu_threshold(const GrayImage& image);
[[nodiscard]] GrayImage binary_threshold(const GrayImage& image, std::uint8_t threshold);
[[nodiscard]] GrayImage otsu_binarize(const GrayImage& image, OtsuResult& result);

} // namespace dip::lab06
