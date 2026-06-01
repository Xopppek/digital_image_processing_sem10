#pragma once

#include "core/gray_image.hpp"

#include <complex>
#include <cstddef>
#include <vector>

namespace dip::lab02 {

using Complex = std::complex<double>;

struct FourierImage {
    std::size_t size{0};
    std::vector<Complex> values;
};

[[nodiscard]] std::size_t next_power_of_two(std::size_t value);

void fft_forward(std::vector<Complex>& data);

[[nodiscard]] std::vector<Complex> forward_fft_signal(const std::vector<double>& samples);
[[nodiscard]] FourierImage forward_fft_image(const GrayImage& image);

[[nodiscard]] GrayImage render_signal_plot(
    const std::vector<double>& samples,
    std::size_t width = 512,
    std::size_t height = 256
);

[[nodiscard]] GrayImage render_signal_log_amplitude_spectrum(
    const std::vector<Complex>& spectrum,
    std::size_t width = 512,
    std::size_t height = 256
);

[[nodiscard]] GrayImage render_image_log_amplitude_spectrum(const FourierImage& spectrum);

} // namespace dip::lab02
