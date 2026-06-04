#include "lab05_nonlinear/rank_filter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dip::lab05 {
namespace {

constexpr std::size_t brightness_levels = 256;
constexpr std::size_t small_aperture_active_size = 32;

void require_valid_aperture(const Aperture& aperture) {
    if (aperture.width == 0 || aperture.height == 0) {
        throw std::invalid_argument("aperture dimensions must be positive");
    }

    if (aperture.width % 2 == 0 || aperture.height % 2 == 0) {
        throw std::invalid_argument("aperture dimensions must be odd");
    }

    if (aperture.mask.size() != aperture.width * aperture.height) {
        throw std::invalid_argument("aperture mask size does not match dimensions");
    }

    if (active_aperture_size(aperture) == 0) {
        throw std::invalid_argument("aperture must include at least one cell");
    }
}

std::vector<std::pair<std::size_t, std::size_t>> active_offsets(const Aperture& aperture) {
    std::vector<std::pair<std::size_t, std::size_t>> offsets;
    offsets.reserve(active_aperture_size(aperture));

    for (std::size_t y = 0; y < aperture.height; ++y) {
        for (std::size_t x = 0; x < aperture.width; ++x) {
            if (aperture.mask[y * aperture.width + x] != 0) {
                offsets.push_back({x, y});
            }
        }
    }

    return offsets;
}

GrayImage::Pixel clamp_to_pixel(const double value) {
    const auto rounded = static_cast<int>(std::round(value));
    return static_cast<GrayImage::Pixel>(std::clamp(rounded, 0, 255));
}

std::array<std::size_t, brightness_levels> collect_histogram(
    const GrayImage& image,
    const std::size_t output_x,
    const std::size_t output_y,
    const std::vector<std::pair<std::size_t, std::size_t>>& offsets
) {
    std::array<std::size_t, brightness_levels> counts{};

    for (const auto& [offset_x, offset_y] : offsets) {
        ++counts[image.at(output_x + offset_x, output_y + offset_y)];
    }

    return counts;
}

GrayImage::Pixel rank_from_histogram(
    const std::array<std::size_t, brightness_levels>& counts,
    const std::size_t rank
) {
    std::size_t accumulated = 0;
    for (std::size_t brightness = 0; brightness < brightness_levels; ++brightness) {
        accumulated += counts[brightness];
        if (rank < accumulated) {
            return static_cast<GrayImage::Pixel>(brightness);
        }
    }

    throw std::runtime_error("failed to calculate rank from histogram");
}

double trimmed_mean_from_histogram(
    const std::array<std::size_t, brightness_levels>& counts,
    const std::size_t trim_each_side,
    const std::size_t kept_count
) {
    std::size_t skipped_low = 0;
    std::size_t kept = 0;
    double sum = 0.0;

    for (std::size_t brightness = 0; brightness < brightness_levels && kept < kept_count; ++brightness) {
        std::size_t count = counts[brightness];
        if (skipped_low < trim_each_side) {
            const std::size_t skip = std::min(count, trim_each_side - skipped_low);
            count -= skip;
            skipped_low += skip;
        }

        const std::size_t take = std::min(count, kept_count - kept);
        sum += static_cast<double>(brightness) * static_cast<double>(take);
        kept += take;
    }

    if (kept != kept_count) {
        throw std::runtime_error("failed to calculate trimmed mean from histogram");
    }

    return sum / static_cast<double>(kept_count);
}

} // namespace

std::size_t active_aperture_size(const Aperture& aperture) {
    return static_cast<std::size_t>(std::count(aperture.mask.begin(), aperture.mask.end(), std::uint8_t{1}));
}

GrayImage rank_filter_valid(const GrayImage& image, const Aperture& aperture, const std::size_t rank) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    require_valid_aperture(aperture);

    const std::size_t active_size = active_aperture_size(aperture);
    if (rank >= active_size) {
        throw std::invalid_argument("rank must be less than active aperture size");
    }

    if (image.width() < aperture.width || image.height() < aperture.height) {
        throw std::invalid_argument("aperture must not be larger than image");
    }

    const std::size_t output_width = image.width() - aperture.width + 1;
    const std::size_t output_height = image.height() - aperture.height + 1;
    const std::vector<std::pair<std::size_t, std::size_t>> offsets = active_offsets(aperture);

    GrayImage result(output_width, output_height);
    std::vector<GrayImage::Pixel> values;
    values.reserve(active_size);
    const bool use_histogram = active_size > small_aperture_active_size;

    for (std::size_t output_y = 0; output_y < output_height; ++output_y) {
        for (std::size_t output_x = 0; output_x < output_width; ++output_x) {
            if (use_histogram) {
                const auto counts = collect_histogram(image, output_x, output_y, offsets);
                result.at(output_x, output_y) = rank_from_histogram(counts, rank);
                continue;
            }

            values.clear();

            for (const auto& [offset_x, offset_y] : offsets) {
                values.push_back(image.at(output_x + offset_x, output_y + offset_y));
            }

            std::nth_element(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(rank), values.end());
            result.at(output_x, output_y) = values[rank];
        }
    }

    return result;
}

GrayImage trimmed_mean_filter_valid(
    const GrayImage& image,
    const Aperture& aperture,
    const std::size_t trimmed_count
) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    require_valid_aperture(aperture);

    const std::size_t active_size = active_aperture_size(aperture);
    if (trimmed_count >= active_size) {
        throw std::invalid_argument("trimmed count must be less than active aperture size");
    }

    if (trimmed_count % 2 != 0) {
        throw std::invalid_argument("trimmed count must be even");
    }

    if (image.width() < aperture.width || image.height() < aperture.height) {
        throw std::invalid_argument("aperture must not be larger than image");
    }

    const std::size_t trim_each_side = trimmed_count / 2;
    const std::size_t kept_count = active_size - trimmed_count;
    const std::size_t output_width = image.width() - aperture.width + 1;
    const std::size_t output_height = image.height() - aperture.height + 1;
    const std::vector<std::pair<std::size_t, std::size_t>> offsets = active_offsets(aperture);

    GrayImage result(output_width, output_height);
    std::vector<GrayImage::Pixel> values;
    values.reserve(active_size);
    const bool use_histogram = active_size > small_aperture_active_size;

    for (std::size_t output_y = 0; output_y < output_height; ++output_y) {
        for (std::size_t output_x = 0; output_x < output_width; ++output_x) {
            if (use_histogram) {
                const auto counts = collect_histogram(image, output_x, output_y, offsets);
                result.at(output_x, output_y) =
                    clamp_to_pixel(trimmed_mean_from_histogram(counts, trim_each_side, kept_count));
                continue;
            }

            values.clear();

            for (const auto& [offset_x, offset_y] : offsets) {
                values.push_back(image.at(output_x + offset_x, output_y + offset_y));
            }

            if (trim_each_side > 0) {
                std::nth_element(
                    values.begin(),
                    values.begin() + static_cast<std::ptrdiff_t>(trim_each_side),
                    values.end()
                );

                std::nth_element(
                    values.begin() + static_cast<std::ptrdiff_t>(trim_each_side),
                    values.begin() + static_cast<std::ptrdiff_t>(active_size - trim_each_side),
                    values.end()
                );
            }

            double sum = 0.0;
            for (std::size_t index = trim_each_side; index < active_size - trim_each_side; ++index) {
                sum += values[index];
            }

            result.at(output_x, output_y) = clamp_to_pixel(sum / static_cast<double>(kept_count));
        }
    }

    return result;
}

} // namespace dip::lab05
