#include "lab04_convolution/high_pass.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace dip::lab04 {
namespace {

constexpr double pi = 3.141592653589793238462643383279502884;

GrayImage::Pixel clamp_to_pixel(const double value) {
    const auto rounded = static_cast<int>(std::round(value));
    return static_cast<GrayImage::Pixel>(std::clamp(rounded, 0, 255));
}

void require_odd_positive_size(const std::size_t size, const char* name) {
    if (size == 0) {
        throw std::invalid_argument(std::string(name) + " must be positive");
    }

    if (size % 2 == 0) {
        throw std::invalid_argument(std::string(name) + " must be odd");
    }
}

void require_valid_response(const ConvolutionResponse& response) {
    if (response.width == 0 || response.height == 0 || response.values.size() != response.width * response.height) {
        throw std::invalid_argument("invalid convolution response dimensions");
    }

    for (const double value : response.values) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("convolution response values must be finite");
        }
    }
}

bool has_zero_crossing(const double first, const double second, const double threshold) {
    if (std::abs(first - second) < threshold) {
        return false;
    }

    return (first < 0.0 && second > 0.0) ||
           (first > 0.0 && second < 0.0);
}

} // namespace

const char* laplacian_kernel_name(const LaplacianKernel kernel) noexcept {
    switch (kernel) {
        case LaplacianKernel::four_neighbor:
            return "four";
        case LaplacianKernel::eight_neighbor:
            return "eight";
    }

    return "unknown";
}

Kernel2D make_laplacian_kernel(const LaplacianKernel kernel) {
    switch (kernel) {
        case LaplacianKernel::four_neighbor:
            return {3, 3, {
                 0.0, -1.0,  0.0,
                -1.0,  4.0, -1.0,
                 0.0, -1.0,  0.0,
            }};
        case LaplacianKernel::eight_neighbor:
            return {3, 3, {
                -1.0, -1.0, -1.0,
                -1.0,  8.0, -1.0,
                -1.0, -1.0, -1.0,
            }};
    }

    throw std::invalid_argument("unknown Laplacian kernel");
}

Kernel2D make_log_kernel(const std::size_t size, const double sigma) {
    require_odd_positive_size(size, "LoG kernel size");

    if (!std::isfinite(sigma) || sigma <= 0.0) {
        throw std::invalid_argument("LoG sigma must be a finite positive value");
    }

    const double sigma2 = sigma * sigma;
    const double sigma4 = sigma2 * sigma2;
    const int half_size = static_cast<int>(size / 2);
    std::vector<double> values;
    values.reserve(size * size);

    double sum = 0.0;
    for (int y = -half_size; y <= half_size; ++y) {
        for (int x = -half_size; x <= half_size; ++x) {
            const double radius2 = static_cast<double>(x * x + y * y);
            const double exponent = std::exp(-radius2 / (2.0 * sigma2));
            const double value = (1.0 / (pi * sigma4)) *
                                 (1.0 - radius2 / (2.0 * sigma2)) *
                                 exponent;
            values.push_back(value);
            sum += value;
        }
    }

    const double mean = sum / static_cast<double>(values.size());
    for (double& value : values) {
        value -= mean;
    }

    return {size, size, std::move(values)};
}

Kernel2D make_sharpening_kernel() {
    constexpr double scale = 1.0 / 5.0;

    return {5, 5, {
         0.0 * scale, -1.0 * scale, -1.0 * scale, -1.0 * scale,  0.0 * scale,
        -1.0 * scale,  1.0 * scale,  1.0 * scale,  1.0 * scale, -1.0 * scale,
        -1.0 * scale,  1.0 * scale,  9.0 * scale,  1.0 * scale, -1.0 * scale,
        -1.0 * scale,  1.0 * scale,  1.0 * scale,  1.0 * scale, -1.0 * scale,
         0.0 * scale, -1.0 * scale, -1.0 * scale, -1.0 * scale,  0.0 * scale,
    }};
}

GrayImage render_response_magnitude(const ConvolutionResponse& response) {
    require_valid_response(response);

    double max_magnitude = 0.0;
    for (const double value : response.values) {
        max_magnitude = std::max(max_magnitude, std::abs(value));
    }

    GrayImage result(response.width, response.height);
    if (max_magnitude == 0.0) {
        return result;
    }

    const double scale = 255.0 / max_magnitude;
    for (std::size_t y = 0; y < response.height; ++y) {
        for (std::size_t x = 0; x < response.width; ++x) {
            const double magnitude = std::abs(response.values[y * response.width + x]);
            result.at(x, y) = clamp_to_pixel(magnitude * scale);
        }
    }

    return result;
}

double automatic_zero_crossing_threshold(const ConvolutionResponse& response) {
    require_valid_response(response);

    double absolute_sum = 0.0;
    for (const double value : response.values) {
        absolute_sum += std::abs(value);
    }

    return 3.0 * absolute_sum / (4.0 * static_cast<double>(response.width * response.height));
}

GrayImage zero_crossing_edges(const ConvolutionResponse& response, const double threshold) {
    require_valid_response(response);

    if (!std::isfinite(threshold) || threshold < 0.0) {
        throw std::invalid_argument("zero-crossing threshold must be a finite non-negative value");
    }

    GrayImage edges(response.width, response.height);
    const auto value_at = [&response](const std::size_t x, const std::size_t y) {
        return response.values[y * response.width + x];
    };

    for (std::size_t y = 0; y < response.height; ++y) {
        for (std::size_t x = 0; x < response.width; ++x) {
            const double center = value_at(x, y);
            bool is_edge = false;

            if (x + 1 < response.width) {
                is_edge = is_edge || has_zero_crossing(center, value_at(x + 1, y), threshold);
            }

            if (x > 0) {
                is_edge = is_edge || has_zero_crossing(center, value_at(x - 1, y), threshold);
            }

            if (y + 1 < response.height) {
                is_edge = is_edge || has_zero_crossing(center, value_at(x, y + 1), threshold);
            }

            if (y > 0) {
                is_edge = is_edge || has_zero_crossing(center, value_at(x, y - 1), threshold);
            }

            edges.at(x, y) = is_edge ? GrayImage::Pixel{255} : GrayImage::Pixel{0};
        }
    }

    return edges;
}

GrayImage laplacian_filter(const GrayImage& image, const LaplacianKernel kernel) {
    return render_response_magnitude(convolve_valid_response(image, make_laplacian_kernel(kernel)));
}

GrayImage log_filter(const GrayImage& image, const std::size_t kernel_size, const double sigma) {
    return render_response_magnitude(log_response(image, kernel_size, sigma));
}

ConvolutionResponse log_response(const GrayImage& image, const std::size_t kernel_size, const double sigma) {
    return convolve_valid_response(image, make_log_kernel(kernel_size, sigma));
}

GrayImage sharpening_filter(const GrayImage& image) {
    return convolve_valid(image, make_sharpening_kernel());
}

} // namespace dip::lab04
