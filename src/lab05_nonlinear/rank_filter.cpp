#include "lab05_nonlinear/rank_filter.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dip::lab05 {
namespace {

void require_valid_aperture(const Aperture& aperture) {
    if (aperture.width == 0 || aperture.height == 0) {
        throw std::invalid_argument("aperture dimensions must be positive");
    }

    if (aperture.width % 2 == 0 || aperture.height % 2 == 0) {
        throw std::invalid_argument("aperture dimensions must be odd");
    }

    if (aperture.mask.size() != aperture.width * aperture.height) {
        throw std::invalid_argument("aperture mask size does not match dimensions");
    }

    if (active_aperture_size(aperture) == 0) {
        throw std::invalid_argument("aperture must include at least one cell");
    }
}

std::vector<std::pair<std::size_t, std::size_t>> active_offsets(const Aperture& aperture) {
    std::vector<std::pair<std::size_t, std::size_t>> offsets;
    offsets.reserve(active_aperture_size(aperture));

    for (std::size_t y = 0; y < aperture.height; ++y) {
        for (std::size_t x = 0; x < aperture.width; ++x) {
            if (aperture.mask[y * aperture.width + x] != 0) {
                offsets.push_back({x, y});
            }
        }
    }

    return offsets;
}

} // namespace

std::size_t active_aperture_size(const Aperture& aperture) {
    return static_cast<std::size_t>(std::count(aperture.mask.begin(), aperture.mask.end(), std::uint8_t{1}));
}

GrayImage rank_filter_valid(const GrayImage& image, const Aperture& aperture, const std::size_t rank) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    require_valid_aperture(aperture);

    const std::size_t active_size = active_aperture_size(aperture);
    if (rank >= active_size) {
        throw std::invalid_argument("rank must be less than active aperture size");
    }

    if (image.width() < aperture.width || image.height() < aperture.height) {
        throw std::invalid_argument("aperture must not be larger than image");
    }

    const std::size_t output_width = image.width() - aperture.width + 1;
    const std::size_t output_height = image.height() - aperture.height + 1;
    const std::vector<std::pair<std::size_t, std::size_t>> offsets = active_offsets(aperture);

    GrayImage result(output_width, output_height);
    std::vector<GrayImage::Pixel> values;
    values.reserve(active_size);

    for (std::size_t output_y = 0; output_y < output_height; ++output_y) {
        for (std::size_t output_x = 0; output_x < output_width; ++output_x) {
            values.clear();

            for (const auto& [offset_x, offset_y] : offsets) {
                values.push_back(image.at(output_x + offset_x, output_y + offset_y));
            }

            std::sort(values.begin(), values.end());
            result.at(output_x, output_y) = values[rank];
        }
    }

    return result;
}

} // namespace dip::lab05
