#pragma once

#include "core/gray_image.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace dip::lab01 {

using Histogram = std::array<std::uint64_t, 256>;

[[nodiscard]] Histogram histogram(const GrayImage& image);
[[nodiscard]] GrayImage render_histogram_image(
    const Histogram& histogram,
    std::size_t width = 512,
    std::size_t height = 256
);

} // namespace dip::lab01
