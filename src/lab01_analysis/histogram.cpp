#include "lab01_analysis/histogram.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace dip::lab01 {

Histogram histogram(const GrayImage& image) {
    Histogram result{};

    for (const GrayImage::Pixel pixel : image.pixels()) {
        ++result[pixel];
    }

    return result;
}

GrayImage render_histogram_image(
    const Histogram& values,
    const std::size_t width,
    const std::size_t height
) {
    if (width < values.size() || height == 0) {
        throw std::invalid_argument("histogram image must be at least 256 pixels wide and non-empty");
    }

    GrayImage image(width, height, std::vector<GrayImage::Pixel>(width * height, 255));
    const std::uint64_t max_count = *std::max_element(values.begin(), values.end());

    if (max_count == 0) {
        return image;
    }

    const std::size_t bin_width = width / values.size();
    const std::size_t usable_width = bin_width * values.size();
    const std::size_t left_padding = (width - usable_width) / 2;

    for (std::size_t bin = 0; bin < values.size(); ++bin) {
        if (values[bin] == 0) {
            continue;
        }

        const double normalized = static_cast<double>(values[bin]) / static_cast<double>(max_count);
        const auto bar_height = std::min<std::size_t>(
            height,
            std::max<std::size_t>(
                1,
                static_cast<std::size_t>(std::round(normalized * static_cast<double>(height)))
            )
        );
        const std::size_t x_begin = left_padding + bin * bin_width;
        const std::size_t x_end = x_begin + bin_width;

        for (std::size_t x = x_begin; x < x_end; ++x) {
            for (std::size_t y_offset = 0; y_offset < bar_height; ++y_offset) {
                const std::size_t y = height - 1 - y_offset;
                image.at(x, y) = 0;
            }
        }
    }

    return image;
}

} // namespace dip::lab01
