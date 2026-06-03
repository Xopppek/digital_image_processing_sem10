#include "lab06_segmentation/otsu.hpp"

#include <array>
#include <stdexcept>

namespace dip::lab06 {
namespace {

constexpr std::size_t brightness_levels = 256;

std::array<std::size_t, brightness_levels> histogram(const GrayImage& image) {
    std::array<std::size_t, brightness_levels> counts{};

    for (const GrayImage::Pixel pixel : image.pixels()) {
        ++counts[pixel];
    }

    return counts;
}

} // namespace

OtsuResult otsu_threshold(const GrayImage& image) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    const auto counts = histogram(image);
    const double total_pixels = static_cast<double>(image.size());

    double total_sum = 0.0;
    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        total_sum += static_cast<double>(brightness) * static_cast<double>(counts[brightness]);
    }

    std::size_t class0_count = 0;
    double class0_sum = 0.0;
    double best_between_class_variance = -1.0;
    OtsuResult best{};

    for (std::size_t threshold = 0; threshold < brightness_levels; ++threshold) {
        class0_count += counts[threshold];
        class0_sum += static_cast<double>(threshold) * static_cast<double>(counts[threshold]);

        const std::size_t class1_count = image.size() - class0_count;
        if (class0_count == 0 || class1_count == 0) {
            continue;
        }

        const double omega0 = static_cast<double>(class0_count) / total_pixels;
        const double omega1 = static_cast<double>(class1_count) / total_pixels;
        const double mean0 = class0_sum / static_cast<double>(class0_count);
        const double mean1 = (total_sum - class0_sum) / static_cast<double>(class1_count);
        const double mean_difference = mean0 - mean1;
        const double between_class_variance = omega0 * omega1 * mean_difference * mean_difference;

        if (between_class_variance > best_between_class_variance) {
            best_between_class_variance = between_class_variance;
            best.threshold = static_cast<std::uint8_t>(threshold);
            best.class_pixel_counts = {class0_count, class1_count};
            best.class_probabilities = {omega0, omega1};
            best.class_means = {mean0, mean1};
            best.between_class_variance = between_class_variance;
        }
    }

    if (best_between_class_variance < 0.0) {
        best.threshold = 0;
        best.class_pixel_counts = {image.size(), 0};
        best.class_probabilities = {1.0, 0.0};
        best.class_means = {total_sum / total_pixels, 0.0};
        best.between_class_variance = 0.0;
        best.within_class_variance = 0.0;
        return best;
    }

    double class0_variance_sum = 0.0;
    double class1_variance_sum = 0.0;
    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        const double value = static_cast<double>(brightness);
        const double count = static_cast<double>(counts[brightness]);
        if (brightness <= best.threshold) {
            const double diff = value - best.class_means[0];
            class0_variance_sum += diff * diff * count;
        } else {
            const double diff = value - best.class_means[1];
            class1_variance_sum += diff * diff * count;
        }
    }

    const double variance0 = best.class_pixel_counts[0] == 0
        ? 0.0
        : class0_variance_sum / static_cast<double>(best.class_pixel_counts[0]);
    const double variance1 = best.class_pixel_counts[1] == 0
        ? 0.0
        : class1_variance_sum / static_cast<double>(best.class_pixel_counts[1]);

    best.within_class_variance =
        best.class_probabilities[0] * variance0 + best.class_probabilities[1] * variance1;

    return best;
}

GrayImage binary_threshold(const GrayImage& image, const std::uint8_t threshold) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    GrayImage result(image.width(), image.height());
    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            result.at(x, y) = image.at(x, y) <= threshold ? 0 : 255;
        }
    }

    return result;
}

GrayImage otsu_binarize(const GrayImage& image, OtsuResult& result) {
    result = otsu_threshold(image);
    return binary_threshold(image, result.threshold);
}

} // namespace dip::lab06
