#pragma once

#include "core/gray_image.hpp"

#include <cstdint>

namespace dip::lab01 {

[[nodiscard]] GrayImage add_gaussian_noise(
    const GrayImage& image,
    double variance,
    std::uint32_t seed
);

[[nodiscard]] GrayImage add_impulse_noise(
    const GrayImage& image,
    double probability,
    std::uint32_t seed
);

} // namespace dip::lab01
