#pragma once

#include "core/gray_image.hpp"
#include "lab04_convolution/convolution.hpp"

#include <cstddef>

namespace dip::lab04 {

enum class LaplacianKernel {
    four_neighbor,
    eight_neighbor,
};

[[nodiscard]] const char* laplacian_kernel_name(LaplacianKernel kernel) noexcept;
[[nodiscard]] Kernel2D make_laplacian_kernel(LaplacianKernel kernel);
[[nodiscard]] Kernel2D make_log_kernel(std::size_t size, double sigma);
[[nodiscard]] GrayImage render_response_magnitude(const ConvolutionResponse& response);
[[nodiscard]] double automatic_zero_crossing_threshold(const ConvolutionResponse& response);
[[nodiscard]] GrayImage zero_crossing_edges(const ConvolutionResponse& response, double threshold);
[[nodiscard]] GrayImage laplacian_filter(const GrayImage& image, LaplacianKernel kernel);
[[nodiscard]] GrayImage log_filter(const GrayImage& image, std::size_t kernel_size, double sigma);
[[nodiscard]] ConvolutionResponse log_response(const GrayImage& image, std::size_t kernel_size, double sigma);

} // namespace dip::lab04
