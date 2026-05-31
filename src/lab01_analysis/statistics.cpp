#include "lab01_analysis/statistics.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace dip::lab01 {
namespace {

constexpr std::size_t brightness_levels = 256;

std::uint64_t total_pixel_count(const Histogram& values) {
    std::uint64_t total_pixels = 0;

    for (const std::uint64_t count : values) {
        total_pixels += count;
    }

    return total_pixels;
}

void require_non_empty(const Histogram& values) {
    if (total_pixel_count(values) == 0) {
        throw std::invalid_argument("statistics require a non-empty image");
    }
}

double probability(const std::uint64_t count, const std::uint64_t total_pixels) {
    return static_cast<double>(count) / static_cast<double>(total_pixels);
}

double nearest_rank_quantile(const Histogram& values, const double probability) {
    require_non_empty(values);

    const std::uint64_t total_pixels = total_pixel_count(values);
    const auto target_rank = static_cast<std::uint64_t>(std::ceil(probability * static_cast<double>(total_pixels)));
    const std::uint64_t rank = target_rank == 0 ? 1 : target_rank;
    std::uint64_t cumulative = 0;

    for (std::size_t value = 0; value < values.size(); ++value) {
        cumulative += values[value];
        if (cumulative >= rank) {
            return static_cast<double>(value);
        }
    }

    return 255.0;
}

}

double mean(const Histogram& values) {
    require_non_empty(values);

    const std::uint64_t total_pixels = total_pixel_count(values);
    double result = 0.0;

    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        const double p = probability(values[brightness], total_pixels);
        result += static_cast<double>(brightness) * p;
    }

    return result;
}

double mean(const GrayImage& image) {
    return mean(histogram(image));
}

double variance(const Histogram& values) {
    require_non_empty(values);

    const double mean_value = mean(values);
    const std::uint64_t total_pixels = total_pixel_count(values);
    double result = 0.0;

    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        const double p = probability(values[brightness], total_pixels);
        const double centered = static_cast<double>(brightness) - mean_value;
        result += centered * centered * p;
    }

    return result;
}

double variance(const GrayImage& image) {
    return variance(histogram(image));
}

Quartiles quartiles(const Histogram& values) {
    return {
        nearest_rank_quantile(values, 0.25),
        nearest_rank_quantile(values, 0.50),
        nearest_rank_quantile(values, 0.75),
    };
}

Quartiles quartiles(const GrayImage& image) {
    return quartiles(histogram(image));
}

double entropy(const Histogram& values) {
    require_non_empty(values);

    const std::uint64_t total_pixels = total_pixel_count(values);
    double result = 0.0;

    for (const std::uint64_t count : values) {
        if (count == 0) {
            continue;
        }

        const double p = probability(count, total_pixels);
        result -= p * std::log2(p);
    }

    return result;
}

double entropy(const GrayImage& image) {
    return entropy(histogram(image));
}

double energy(const Histogram& values) {
    require_non_empty(values);

    const std::uint64_t total_pixels = total_pixel_count(values);
    double result = 0.0;

    for (const std::uint64_t count : values) {
        const double p = probability(count, total_pixels);
        result += p * p;
    }

    return result;
}

double energy(const GrayImage& image) {
    return energy(histogram(image));
}

double skewness(const Histogram& values, const double mean_value, const double variance_value) {
    require_non_empty(values);

    if (variance_value == 0.0) {
        return 0.0;
    }

    const std::uint64_t total_pixels = total_pixel_count(values);
    const double sigma = std::sqrt(variance_value);
    double moment3 = 0.0;

    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        const double p = probability(values[brightness], total_pixels);
        const double centered = static_cast<double>(brightness) - mean_value;
        moment3 += centered * centered * centered * p;
    }

    return moment3 / (sigma * sigma * sigma);
}

double skewness(const GrayImage& image) {
    const Histogram values = histogram(image);
    const double mean_value = mean(values);
    const double variance_value = variance(values);
    return skewness(values, mean_value, variance_value);
}

double kurtosis(const Histogram& values, const double mean_value, const double variance_value) {
    require_non_empty(values);

    if (variance_value == 0.0) {
        return 0.0;
    }

    const std::uint64_t total_pixels = total_pixel_count(values);
    double moment4 = 0.0;

    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        const double p = probability(values[brightness], total_pixels);
        const double centered = static_cast<double>(brightness) - mean_value;
        const double centered2 = centered * centered;
        moment4 += centered2 * centered2 * p;
    }

    return moment4 / (variance_value * variance_value) - 3.0;
}

double kurtosis(const GrayImage& image) {
    const Histogram values = histogram(image);
    const double mean_value = mean(values);
    const double variance_value = variance(values);
    return kurtosis(values, mean_value, variance_value);
}

ImageStatistics calculate_statistics(const GrayImage& image) {
    ImageStatistics result;
    result.histogram = histogram(image);
    result.mean = mean(result.histogram);
    result.variance = variance(result.histogram);
    result.quartiles = quartiles(result.histogram);
    result.entropy = entropy(result.histogram);
    result.energy = energy(result.histogram);
    result.skewness = skewness(result.histogram, result.mean, result.variance);
    result.kurtosis = kurtosis(result.histogram, result.mean, result.variance);

    return result;
}

} // namespace dip::lab01
