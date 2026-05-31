#include "lab01_analysis/noise.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace dip::lab01 {
namespace {

class BoxMullerGenerator {
public:
    explicit BoxMullerGenerator(const std::uint32_t seed)
        : random_engine_(seed),
          uniform_distribution_(-1.0, 1.0) {}

    double next_standard_normal() {
        if (has_saved_value_) {
            has_saved_value_ = false;
            return saved_value_;
        }

        while (true) {
            const double x = uniform_distribution_(random_engine_);
            const double y = uniform_distribution_(random_engine_);
            const double s = x * x + y * y;

            if (s > 1.0 || s == 0.0) {
                continue;
            }

            const double multiplier = std::sqrt((-2.0 * std::log(s)) / s);
            saved_value_ = y * multiplier;
            has_saved_value_ = true;

            return x * multiplier;
        }
    }

private:
    std::mt19937 random_engine_;
    std::uniform_real_distribution<double> uniform_distribution_;
    bool has_saved_value_{false};
    double saved_value_{0.0};
};

GrayImage::Pixel clamp_to_pixel(const double value) {
    const auto rounded = static_cast<int>(std::round(value));
    return static_cast<GrayImage::Pixel>(std::clamp(rounded, 0, 255));
}

} // namespace

GrayImage add_gaussian_noise(
    const GrayImage& image,
    const double variance,
    const std::uint32_t seed
) {
    if (image.empty()) {
        throw std::invalid_argument("gaussian noise requires a non-empty image");
    }

    if (variance < 0.0) {
        throw std::invalid_argument("gaussian noise variance must be non-negative");
    }

    const double sigma = std::sqrt(variance);
    BoxMullerGenerator generator(seed);
    std::vector<GrayImage::Pixel> pixels;
    pixels.reserve(image.size());

    for (const GrayImage::Pixel pixel : image.pixels()) {
        const double noise = sigma * generator.next_standard_normal();
        pixels.push_back(clamp_to_pixel(static_cast<double>(pixel) + noise));
    }

    return GrayImage(image.width(), image.height(), std::move(pixels));
}

} // namespace dip::lab01
