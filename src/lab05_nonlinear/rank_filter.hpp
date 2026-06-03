#pragma once

#include "core/gray_image.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dip::lab05 {

struct Aperture {
    std::size_t width{0};
    std::size_t height{0};
    std::vector<std::uint8_t> mask;
};

[[nodiscard]] std::size_t active_aperture_size(const Aperture& aperture);
[[nodiscard]] GrayImage rank_filter_valid(const GrayImage& image, const Aperture& aperture, std::size_t rank);

} // namespace dip::lab05
