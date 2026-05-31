#include "lab01_analysis/cooccurrence.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace dip::lab01 {
namespace {

std::size_t cooccurrence_index(const GrayImage::Pixel first, const GrayImage::Pixel second) {
    return static_cast<std::size_t>(first) * cooccurrence_levels + static_cast<std::size_t>(second);
}

bool inside_image(const std::int64_t row, const std::int64_t column, const GrayImage& image) {
    return row >= 0 &&
           column >= 0 &&
           row < static_cast<std::int64_t>(image.height()) &&
           column < static_cast<std::int64_t>(image.width());
}

} // namespace

CooccurrenceMatrix cooccurrence_matrix(
    const GrayImage& image,
    const int row_offset,
    const int column_offset
) {
    if (image.empty()) {
        throw std::invalid_argument("co-occurrence matrix requires a non-empty image");
    }

    CooccurrenceMatrix result;
    result.row_offset = row_offset;
    result.column_offset = column_offset;

    for (std::size_t row = 0; row < image.height(); ++row) {
        for (std::size_t column = 0; column < image.width(); ++column) {
            const auto neighbor_row = static_cast<std::int64_t>(row) + row_offset;
            const auto neighbor_column = static_cast<std::int64_t>(column) + column_offset;

            if (!inside_image(neighbor_row, neighbor_column, image)) {
                continue;
            }

            const GrayImage::Pixel first = image.at(column, row);
            const GrayImage::Pixel second = image.at(
                static_cast<std::size_t>(neighbor_column),
                static_cast<std::size_t>(neighbor_row)
            );

            ++result.counts[cooccurrence_index(first, second)];
            ++result.total_pairs;
        }
    }

    return result;
}

std::uint64_t cooccurrence_count(
    const CooccurrenceMatrix& matrix,
    const GrayImage::Pixel first,
    const GrayImage::Pixel second
) {
    return matrix.counts[cooccurrence_index(first, second)];
}

double cooccurrence_probability(
    const CooccurrenceMatrix& matrix,
    const GrayImage::Pixel first,
    const GrayImage::Pixel second
) {
    if (matrix.total_pairs == 0) {
        throw std::invalid_argument("co-occurrence probability requires at least one pixel pair");
    }

    return static_cast<double>(cooccurrence_count(matrix, first, second)) /
           static_cast<double>(matrix.total_pairs);
}

double cooccurrence_energy(const CooccurrenceMatrix& matrix) {
    double result = 0.0;

    for (std::size_t first = 0; first < cooccurrence_levels; ++first) {
        for (std::size_t second = 0; second < cooccurrence_levels; ++second) {
            const double probability = cooccurrence_probability(
                matrix,
                static_cast<GrayImage::Pixel>(first),
                static_cast<GrayImage::Pixel>(second)
            );
            result += probability * probability;
        }
    }

    return result;
}

GrayImage render_cooccurrence_image(const CooccurrenceMatrix& matrix) {
    GrayImage image(
        cooccurrence_levels,
        cooccurrence_levels,
        std::vector<GrayImage::Pixel>(cooccurrence_levels * cooccurrence_levels, 255)
    );
    const std::uint64_t max_count = *std::max_element(matrix.counts.begin(), matrix.counts.end());

    if (max_count == 0) {
        return image;
    }

    for (std::size_t first = 0; first < cooccurrence_levels; ++first) {
        for (std::size_t second = 0; second < cooccurrence_levels; ++second) {
            const std::uint64_t count = matrix.counts[first * cooccurrence_levels + second];
            const double normalized = static_cast<double>(count) / static_cast<double>(max_count);
            const auto value = static_cast<GrayImage::Pixel>(
                255 - static_cast<int>(std::round(normalized * 255.0))
            );

            image.at(second, first) = value;
        }
    }

    return image;
}

} // namespace dip::lab01
