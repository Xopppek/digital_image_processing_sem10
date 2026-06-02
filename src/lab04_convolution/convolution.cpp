#include "lab04_convolution/convolution.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace dip::lab04 {
namespace {

void require_valid_kernel(const Kernel2D& kernel) {
    if (kernel.width == 0 || kernel.height == 0) {
        throw std::invalid_argument("kernel dimensions must be positive");
    }

    if (kernel.width % 2 == 0 || kernel.height % 2 == 0) {
        throw std::invalid_argument("kernel dimensions must be odd");
    }

    if (kernel.values.size() != kernel.width * kernel.height) {
        throw std::invalid_argument("kernel value count does not match dimensions");
    }

    for (const double value : kernel.values) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("kernel values must be finite");
        }
    }
}

GrayImage::Pixel clamp_to_pixel(const double value) {
    const auto rounded = static_cast<int>(std::round(value));
    return static_cast<GrayImage::Pixel>(std::clamp(rounded, 0, 255));
}

}

ConvolutionResponse convolve_valid_response(const GrayImage& image, const Kernel2D& kernel) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    require_valid_kernel(kernel);

    if (image.width() < kernel.width || image.height() < kernel.height) {
        throw std::invalid_argument("kernel must not be larger than image");
    }

    const std::size_t half_width = kernel.width / 2;
    const std::size_t half_height = kernel.height / 2;
    const std::size_t output_width = image.width() - kernel.width + 1;
    const std::size_t output_height = image.height() - kernel.height + 1;

    ConvolutionResponse response;
    response.width = output_width;
    response.height = output_height;
    response.values.assign(output_width * output_height, 0.0);

    for (std::size_t output_row = 0; output_row < output_height; ++output_row) {
        const std::size_t source_center_row = output_row + half_height;

        for (std::size_t output_column = 0; output_column < output_width; ++output_column) {
            const std::size_t source_center_column = output_column + half_width;
            double sum = 0.0;

            for (std::size_t kernel_row = 0; kernel_row < kernel.height; ++kernel_row) {
                const std::size_t source_row = source_center_row + kernel_row - half_height;

                for (std::size_t kernel_column = 0; kernel_column < kernel.width; ++kernel_column) {
                    const std::size_t source_column = source_center_column + kernel_column - half_width;
                    const double pixel = image.at(source_column, source_row);
                    const double weight = kernel.values[kernel_row * kernel.width + kernel_column];
                    sum += pixel * weight;
                }
            }

            response.values[output_row * output_width + output_column] = sum;
        }
    }

    return response;
}

GrayImage convolve_valid(const GrayImage& image, const Kernel2D& kernel) {
    const ConvolutionResponse response = convolve_valid_response(image, kernel);
    GrayImage result(response.width, response.height);

    for (std::size_t y = 0; y < response.height; ++y) {
        for (std::size_t x = 0; x < response.width; ++x) {
            result.at(x, y) = clamp_to_pixel(response.values[y * response.width + x]);
        }
    }

    return result;
}

} // namespace dip::lab04
