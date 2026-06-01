#include "lab02_fourier/fft.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace dip::lab02 {
namespace {

constexpr double pi = 3.141592653589793238462643383279502884;

bool is_power_of_two(const std::size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

std::size_t log2_power_of_two(const std::size_t value) {
    std::size_t result = 0;
    std::size_t current = value;

    while (current > 1) {
        current >>= 1;
        ++result;
    }

    return result;
}

void complex_bit_reverse(std::vector<Complex>& data) {
    if (data.size() < 2) {
        return;
    }

    const std::size_t middle = data.size() / 2;
    const std::size_t reversed_size = data.size() - 1;
    std::size_t j = 0;

    for (std::size_t i = 0; i < reversed_size; ++i) {
        if (i < j) {
            std::swap(data[i], data[j]);
        }

        std::size_t k = middle;
        while (k <= j) {
            j -= k;
            k /= 2;
        }

        j += k;
    }
}

std::vector<Complex> shift_1d(const std::vector<Complex>& values) {
    std::vector<Complex> result(values.size());
    const std::size_t half = values.size() / 2;

    for (std::size_t i = 0; i < values.size(); ++i) {
        result[i] = values[(i + half) % values.size()];
    }

    return result;
}

std::vector<Complex> shift_2d(const FourierImage& spectrum) {
    std::vector<Complex> result(spectrum.values.size());
    const std::size_t half = spectrum.size / 2;

    for (std::size_t row = 0; row < spectrum.size; ++row) {
        for (std::size_t column = 0; column < spectrum.size; ++column) {
            const std::size_t source_row = (row + half) % spectrum.size;
            const std::size_t source_column = (column + half) % spectrum.size;
            result[row * spectrum.size + column] =
                spectrum.values[source_row * spectrum.size + source_column];
        }
    }

    return result;
}

std::vector<double> log_amplitudes(const std::vector<Complex>& values) {
    std::vector<double> result;
    result.reserve(values.size());

    for (const Complex& value : values) {
        result.push_back(std::log1p(std::abs(value)));
    }

    return result;
}

GrayImage::Pixel scale_to_pixel(const double value, const double max_value) {
    if (max_value <= 0.0) {
        return 0;
    }

    const auto scaled = static_cast<int>(std::round(value / max_value * 255.0));
    return static_cast<GrayImage::Pixel>(std::clamp(scaled, 0, 255));
}

std::size_t sample_x(const std::size_t index, const std::size_t sample_count, const std::size_t width) {
    if (sample_count <= 1) {
        return 0;
    }

    return static_cast<std::size_t>(
        std::round(static_cast<double>(index) * static_cast<double>(width - 1) /
                   static_cast<double>(sample_count - 1))
    );
}

std::size_t signal_y(
    const double value,
    const double min_value,
    const double max_value,
    const std::size_t height
) {
    const double position = (max_value - value) / (max_value - min_value);
    const auto row = static_cast<int>(std::round(position * static_cast<double>(height - 1)));
    return static_cast<std::size_t>(std::clamp(row, 0, static_cast<int>(height - 1)));
}

void draw_line(
    GrayImage& image,
    const std::size_t x0,
    const std::size_t y0,
    const std::size_t x1,
    const std::size_t y1,
    const GrayImage::Pixel color
) {
    int current_x = static_cast<int>(x0);
    int current_y = static_cast<int>(y0);
    const int target_x = static_cast<int>(x1);
    const int target_y = static_cast<int>(y1);
    const int dx = std::abs(target_x - current_x);
    const int sx = current_x < target_x ? 1 : -1;
    const int dy = -std::abs(target_y - current_y);
    const int sy = current_y < target_y ? 1 : -1;
    int error = dx + dy;

    while (true) {
        if (
            current_x >= 0 &&
            current_y >= 0 &&
            current_x < static_cast<int>(image.width()) &&
            current_y < static_cast<int>(image.height())
        ) {
            image.at(static_cast<std::size_t>(current_x), static_cast<std::size_t>(current_y)) = color;
        }

        if (current_x == target_x && current_y == target_y) {
            break;
        }

        const int doubled_error = 2 * error;
        if (doubled_error >= dy) {
            error += dy;
            current_x += sx;
        }
        if (doubled_error <= dx) {
            error += dx;
            current_y += sy;
        }
    }
}

}

std::size_t next_power_of_two(const std::size_t value) {
    if (value == 0) {
        throw std::invalid_argument("next power of two requires a positive value");
    }

    std::size_t result = 1;
    while (result < value) {
        result <<= 1;
    }

    return result;
}

void fft_forward(std::vector<Complex>& data) {
    if (!is_power_of_two(data.size())) {
        throw std::invalid_argument("FFT size must be a power of two");
    }

    complex_bit_reverse(data);

    std::size_t points_in_right_dft = 1;
    const std::size_t stage_count = log2_power_of_two(data.size());

    for (std::size_t stage = 1; stage <= stage_count; ++stage) {
        const std::size_t points_in_left_dft = points_in_right_dft;
        points_in_right_dft *= 2;

        Complex twiddle(1.0, 0.0);
        const double trig_arg = pi / static_cast<double>(points_in_left_dft);
        const Complex w_factor(std::cos(trig_arg), -std::sin(trig_arg));

        for (std::size_t butterfly_pos = 0; butterfly_pos < points_in_left_dft; ++butterfly_pos) {
            for (
                std::size_t top_node = butterfly_pos;
                top_node < data.size();
                top_node += points_in_right_dft
            ) {
                const std::size_t bottom_node = top_node + points_in_left_dft;
                const Complex temp = data[bottom_node] * twiddle;
                data[bottom_node] = data[top_node] - temp;
                data[top_node] += temp;
            }

            twiddle *= w_factor;
        }
    }
}

std::vector<Complex> forward_fft_signal(const std::vector<double>& samples) {
    if (samples.empty()) {
        throw std::invalid_argument("signal FFT requires at least one sample");
    }

    std::vector<Complex> data(next_power_of_two(samples.size()), Complex(0.0, 0.0));

    for (std::size_t i = 0; i < samples.size(); ++i) {
        data[i] = Complex(samples[i], 0.0);
    }

    fft_forward(data);
    return data;
}

GrayImage render_signal_plot(
    const std::vector<double>& samples,
    const std::size_t width,
    const std::size_t height
) {
    if (samples.empty() || width == 0 || height == 0) {
        throw std::invalid_argument("signal plot rendering requires non-empty data and image size");
    }

    const auto [min_sample, max_sample] = std::minmax_element(samples.begin(), samples.end());
    double min_value = std::min(*min_sample, 0.0);
    double max_value = std::max(*max_sample, 0.0);

    if (min_value == max_value) {
        min_value -= 1.0;
        max_value += 1.0;
    }

    GrayImage image(width, height, std::vector<GrayImage::Pixel>(width * height, 0));
    const std::size_t zero_row = signal_y(0.0, min_value, max_value, height);

    for (std::size_t column = 0; column < width; ++column) {
        image.at(column, zero_row) = 64;
    }

    std::size_t previous_x = sample_x(0, samples.size(), width);
    std::size_t previous_y = signal_y(samples.front(), min_value, max_value, height);
    image.at(previous_x, previous_y) = 255;

    for (std::size_t i = 1; i < samples.size(); ++i) {
        const std::size_t x = sample_x(i, samples.size(), width);
        const std::size_t y = signal_y(samples[i], min_value, max_value, height);
        draw_line(image, previous_x, previous_y, x, y, 255);
        previous_x = x;
        previous_y = y;
    }

    return image;
}

FourierImage forward_fft_image(const GrayImage& image) {
    if (image.empty()) {
        throw std::invalid_argument("image FFT requires a non-empty image");
    }

    const std::size_t size = next_power_of_two(std::max(image.width(), image.height()));
    FourierImage result;
    result.size = size;
    result.values.assign(size * size, Complex(0.0, 0.0));

    for (std::size_t row = 0; row < image.height(); ++row) {
        for (std::size_t column = 0; column < image.width(); ++column) {
            result.values[row * size + column] = Complex(image.at(column, row), 0.0);
        }
    }

    std::vector<Complex> line(size);

    for (std::size_t row = 0; row < size; ++row) {
        for (std::size_t column = 0; column < size; ++column) {
            line[column] = result.values[row * size + column];
        }

        fft_forward(line);

        for (std::size_t column = 0; column < size; ++column) {
            result.values[row * size + column] = line[column];
        }
    }

    for (std::size_t column = 0; column < size; ++column) {
        for (std::size_t row = 0; row < size; ++row) {
            line[row] = result.values[row * size + column];
        }

        fft_forward(line);

        for (std::size_t row = 0; row < size; ++row) {
            result.values[row * size + column] = line[row];
        }
    }

    return result;
}

GrayImage render_signal_log_amplitude_spectrum(
    const std::vector<Complex>& spectrum,
    const std::size_t width,
    const std::size_t height
) {
    if (spectrum.empty() || width == 0 || height == 0) {
        throw std::invalid_argument("signal spectrum rendering requires non-empty data and image size");
    }

    const std::vector<double> amplitudes = log_amplitudes(shift_1d(spectrum));
    const double max_amplitude = *std::max_element(amplitudes.begin(), amplitudes.end());
    GrayImage image(width, height, std::vector<GrayImage::Pixel>(width * height, 0));

    for (std::size_t column = 0; column < width; ++column) {
        const std::size_t begin = column * amplitudes.size() / width;
        const std::size_t end = std::max(begin + 1, (column + 1) * amplitudes.size() / width);
        double column_amplitude = 0.0;

        for (std::size_t i = begin; i < end && i < amplitudes.size(); ++i) {
            column_amplitude = std::max(column_amplitude, amplitudes[i]);
        }

        const auto bar_height = static_cast<std::size_t>(
            std::round(static_cast<double>(scale_to_pixel(column_amplitude, max_amplitude)) / 255.0 *
                       static_cast<double>(height))
        );

        for (std::size_t y_offset = 0; y_offset < bar_height && y_offset < height; ++y_offset) {
            image.at(column, height - 1 - y_offset) = 255;
        }
    }

    return image;
}

GrayImage render_image_log_amplitude_spectrum(const FourierImage& spectrum) {
    if (spectrum.size == 0 || spectrum.values.size() != spectrum.size * spectrum.size) {
        throw std::invalid_argument("image spectrum rendering requires a valid square spectrum");
    }

    const std::vector<double> amplitudes = log_amplitudes(shift_2d(spectrum));
    const double max_amplitude = *std::max_element(amplitudes.begin(), amplitudes.end());
    std::vector<GrayImage::Pixel> pixels;
    pixels.reserve(amplitudes.size());

    for (const double amplitude : amplitudes) {
        pixels.push_back(scale_to_pixel(amplitude, max_amplitude));
    }

    return GrayImage(spectrum.size, spectrum.size, std::move(pixels));
}

} // namespace dip::lab02
