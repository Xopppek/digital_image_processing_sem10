#pragma once

#include "core/gray_image.hpp"

namespace dip::lab01 {

[[nodiscard]] double mean_squared_error(
    const GrayImage& original,
    const GrayImage& distorted
);

[[nodiscard]] double peak_signal_to_noise_ratio(
    const GrayImage& original,
    const GrayImage& distorted
);

} // namespace dip::lab01
