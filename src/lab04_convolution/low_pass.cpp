#include "lab04_convolution/low_pass.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace dip::lab04 {

Kernel2D make_average_low_pass_kernel(const std::size_t size) {
    if (size == 0) {
        throw std::invalid_argument("low-pass kernel size must be positive");
    }

    if (size % 2 == 0) {
        throw std::invalid_argument("low-pass kernel size must be odd");
    }

    const double value = 1.0 / static_cast<double>(size * size);
    return {size, size, std::vector<double>(size * size, value)};
}

GrayImage average_low_pass_filter(const GrayImage& image, const std::size_t kernel_size) {
    return convolve_valid(image, make_average_low_pass_kernel(kernel_size));
}

GrayImage thresholded_average_filter(
    const GrayImage& image,
    const std::size_t kernel_size,
    const double threshold
) {
    if (!std::isfinite(threshold) || threshold < 0.0) {
        throw std::invalid_argument("threshold must be a finite non-negative value");
    }

    const GrayImage smoothed = average_low_pass_filter(image, kernel_size);
    const std::size_t half_size = kernel_size / 2;
    GrayImage result(smoothed.width(), smoothed.height());

    for (std::size_t y = 0; y < smoothed.height(); ++y) {
        for (std::size_t x = 0; x < smoothed.width(); ++x) {
            const GrayImage::Pixel original_pixel = image.at(x + half_size, y + half_size);
            const GrayImage::Pixel smoothed_pixel = smoothed.at(x, y);
            const double difference = std::abs(
                static_cast<double>(smoothed_pixel) - static_cast<double>(original_pixel)
            );

            result.at(x, y) = difference >= threshold ? smoothed_pixel : original_pixel;
        }
    }

    return result;
}

} // namespace dip::lab04
