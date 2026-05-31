#pragma once

#include "core/gray_image.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace dip::lab01 {

constexpr std::size_t cooccurrence_levels = 256;

struct CooccurrenceMatrix {
    std::array<std::uint64_t, cooccurrence_levels * cooccurrence_levels> counts{};
    std::uint64_t total_pairs{0};
    int row_offset{0};
    int column_offset{0};
};

[[nodiscard]] CooccurrenceMatrix cooccurrence_matrix(
    const GrayImage& image,
    int row_offset,
    int column_offset
);

[[nodiscard]] std::uint64_t cooccurrence_count(
    const CooccurrenceMatrix& matrix,
    GrayImage::Pixel first,
    GrayImage::Pixel second
);

[[nodiscard]] double cooccurrence_probability(
    const CooccurrenceMatrix& matrix,
    GrayImage::Pixel first,
    GrayImage::Pixel second
);

[[nodiscard]] double cooccurrence_energy(const CooccurrenceMatrix& matrix);
[[nodiscard]] GrayImage render_cooccurrence_image(const CooccurrenceMatrix& matrix);

} // namespace dip::lab01
