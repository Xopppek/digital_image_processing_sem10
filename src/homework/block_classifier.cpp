#include "homework/block_classifier.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dip::homework {
namespace {

constexpr GrayImage::Pixel white_threshold = 245;
constexpr GrayImage::Pixel dark_threshold = 128;
constexpr GrayImage::Pixel midtone_low = 64;
constexpr GrayImage::Pixel normalized_white_threshold = 205;
constexpr GrayImage::Pixel normalized_dark_threshold = 85;
constexpr GrayImage::Pixel normalized_midtone_low = 80;
constexpr GrayImage::Pixel normalized_midtone_high = 205;
constexpr double background_max_nonwhite_ratio = 0.015;
constexpr double text_min_nonwhite_ratio = 0.015;
constexpr double text_min_white_ratio = 0.70;
constexpr double text_max_midtone_ratio = 0.20;
constexpr double text_max_dark_ratio = 0.30;
constexpr double image_max_white_ratio = 0.70;
constexpr double image_min_nonwhite_ratio = 0.45;
constexpr double image_min_midtone_ratio = 0.20;
constexpr double image_min_dark_ratio = 0.14;
constexpr double overlay_alpha = 0.35;

RgbPixel blend(const GrayImage::Pixel source, const RgbPixel overlay, const double alpha) {
    const auto blend_channel = [source, alpha](const std::uint8_t channel) {
        const double value = static_cast<double>(source) * (1.0 - alpha) + static_cast<double>(channel) * alpha;
        return static_cast<std::uint8_t>(std::clamp(static_cast<int>(std::round(value)), 0, 255));
    };

    return {
        blend_channel(overlay.red),
        blend_channel(overlay.green),
        blend_channel(overlay.blue),
    };
}

BlockClass classify_by_features(
    const double white_ratio,
    const double dark_ratio,
    const double nonwhite_ratio,
    const double midtone_ratio,
    const double normalized_white_ratio,
    const double normalized_dark_ratio,
    const double normalized_midtone_ratio,
    const double text_projection_score
) {
    if (nonwhite_ratio < background_max_nonwhite_ratio) {
        return BlockClass::background;
    }

    const bool raw_text_like =
        white_ratio >= text_min_white_ratio &&
        nonwhite_ratio >= text_min_nonwhite_ratio &&
        midtone_ratio < text_max_midtone_ratio &&
        dark_ratio < text_max_dark_ratio;

    const bool normalized_text_like =
        normalized_white_ratio >= 0.45 &&
        normalized_dark_ratio >= 0.02 &&
        normalized_dark_ratio <= 0.45 &&
        normalized_midtone_ratio <= 0.45 &&
        text_projection_score >= 0.22;

    const bool dense_newspaper_text_like =
        white_ratio >= 0.50 &&
        white_ratio <= 0.72 &&
        nonwhite_ratio <= 0.55 &&
        dark_ratio <= 0.35 &&
        midtone_ratio <= 0.30 &&
        normalized_white_ratio >= 0.60 &&
        normalized_dark_ratio <= 0.34 &&
        normalized_midtone_ratio <= 0.18 &&
        text_projection_score >= 0.06;

    if (
        raw_text_like ||
        normalized_text_like ||
        dense_newspaper_text_like
    ) {
        return BlockClass::text;
    }

    if (
        white_ratio < image_max_white_ratio ||
        nonwhite_ratio >= image_min_nonwhite_ratio ||
        midtone_ratio >= image_min_midtone_ratio ||
        dark_ratio >= image_min_dark_ratio
    ) {
        return BlockClass::image;
    }

    return BlockClass::background;
}

GrayImage::Pixel normalize_pixel_for_block(
    const GrayImage::Pixel pixel,
    const GrayImage::Pixel min_pixel,
    const GrayImage::Pixel max_pixel
) {
    if (max_pixel <= min_pixel) {
        return pixel;
    }

    const int range = static_cast<int>(max_pixel) - static_cast<int>(min_pixel);
    const int shifted = static_cast<int>(pixel) - static_cast<int>(min_pixel);
    return static_cast<GrayImage::Pixel>((shifted * 255 + range / 2) / range);
}

double row_projection_score(const std::vector<std::size_t>& row_dark_counts, const std::size_t block_width) {
    if (row_dark_counts.empty() || block_width == 0) {
        return 0.0;
    }

    std::size_t active_rows = 0;
    double sum = 0.0;
    double sum_squares = 0.0;
    const double active_threshold = std::max(1.0, static_cast<double>(block_width) * 0.08);

    for (const std::size_t count : row_dark_counts) {
        const double ratio = static_cast<double>(count) / static_cast<double>(block_width);
        sum += ratio;
        sum_squares += ratio * ratio;
        if (static_cast<double>(count) >= active_threshold) {
            ++active_rows;
        }
    }

    const double row_count = static_cast<double>(row_dark_counts.size());
    const double mean = sum / row_count;
    const double variance = std::max(0.0, sum_squares / row_count - mean * mean);
    const double active_ratio = static_cast<double>(active_rows) / row_count;

    if (active_ratio < 0.08 || active_ratio > 0.90) {
        return 0.0;
    }

    return std::sqrt(variance) * (1.0 - std::abs(active_ratio - 0.45));
}

std::size_t grid_index(const std::size_t column, const std::size_t row, const std::size_t grid_width) {
    return row * grid_width + column;
}

std::vector<std::size_t> image_neighbors(
    const BlockClassification& classification,
    const std::size_t column,
    const std::size_t row
) {
    std::vector<std::size_t> result;
    const std::size_t grid_width = classification.grid_width;
    const std::size_t grid_height = classification.grid_height;

    const auto add_if_image = [&](const std::size_t neighbor_column, const std::size_t neighbor_row) {
        const std::size_t index = grid_index(neighbor_column, neighbor_row, grid_width);
        if (classification.blocks[index].block_class == BlockClass::image) {
            result.push_back(index);
        }
    };

    if (column > 0) {
        add_if_image(column - 1, row);
    }
    if (column + 1 < grid_width) {
        add_if_image(column + 1, row);
    }
    if (row > 0) {
        add_if_image(column, row - 1);
    }
    if (row + 1 < grid_height) {
        add_if_image(column, row + 1);
    }

    return result;
}

BlockClass fallback_class_for_removed_image_block(const BlockInfo& block) {
    if (block.nonwhite_ratio < background_max_nonwhite_ratio) {
        return BlockClass::background;
    }

    return BlockClass::text;
}

} // namespace

const char* block_class_name(const BlockClass block_class) noexcept {
    switch (block_class) {
        case BlockClass::background:
            return "background";
        case BlockClass::text:
            return "text";
        case BlockClass::image:
            return "image";
    }

    return "unknown";
}

BlockClassification classify_blocks(const GrayImage& image, const std::size_t block_size) {
    if (image.empty()) {
        throw std::invalid_argument("image must not be empty");
    }

    if (block_size == 0) {
        throw std::invalid_argument("block size must be positive");
    }

    BlockClassification result;
    result.image_width = image.width();
    result.image_height = image.height();
    result.block_size = block_size;
    result.grid_width = (image.width() + block_size - 1) / block_size;
    result.grid_height = (image.height() + block_size - 1) / block_size;

    for (std::size_t y = 0; y < image.height(); y += block_size) {
        for (std::size_t x = 0; x < image.width(); x += block_size) {
            BlockInfo block;
            block.x = x;
            block.y = y;
            block.width = std::min(block_size, image.width() - x);
            block.height = std::min(block_size, image.height() - y);

            const double total_pixels = static_cast<double>(block.width * block.height);
            double sum = 0.0;
            double sum_squares = 0.0;
            std::size_t white_count = 0;
            std::size_t dark_count = 0;
            std::size_t nonwhite_count = 0;
            std::size_t midtone_count = 0;
            GrayImage::Pixel min_pixel = 255;
            GrayImage::Pixel max_pixel = 0;

            for (std::size_t dy = 0; dy < block.height; ++dy) {
                for (std::size_t dx = 0; dx < block.width; ++dx) {
                    const GrayImage::Pixel pixel = image.at(x + dx, y + dy);
                    const double value = static_cast<double>(pixel);
                    sum += value;
                    sum_squares += value * value;
                    min_pixel = std::min(min_pixel, pixel);
                    max_pixel = std::max(max_pixel, pixel);

                    if (pixel >= white_threshold) {
                        ++white_count;
                    } else {
                        ++nonwhite_count;
                    }

                    if (pixel <= dark_threshold) {
                        ++dark_count;
                    }

                    if (pixel >= midtone_low && pixel < white_threshold) {
                        ++midtone_count;
                    }
                }
            }

            std::size_t normalized_white_count = 0;
            std::size_t normalized_dark_count = 0;
            std::size_t normalized_midtone_count = 0;
            std::vector<std::size_t> row_dark_counts(block.height, 0);

            for (std::size_t dy = 0; dy < block.height; ++dy) {
                for (std::size_t dx = 0; dx < block.width; ++dx) {
                    const GrayImage::Pixel normalized = normalize_pixel_for_block(
                        image.at(x + dx, y + dy),
                        min_pixel,
                        max_pixel
                    );

                    if (normalized >= normalized_white_threshold) {
                        ++normalized_white_count;
                    }

                    if (normalized <= normalized_dark_threshold) {
                        ++normalized_dark_count;
                        ++row_dark_counts[dy];
                    }

                    if (normalized >= normalized_midtone_low && normalized < normalized_midtone_high) {
                        ++normalized_midtone_count;
                    }
                }
            }

            block.white_ratio = static_cast<double>(white_count) / total_pixels;
            block.dark_ratio = static_cast<double>(dark_count) / total_pixels;
            block.nonwhite_ratio = static_cast<double>(nonwhite_count) / total_pixels;
            block.midtone_ratio = static_cast<double>(midtone_count) / total_pixels;
            block.normalized_white_ratio = static_cast<double>(normalized_white_count) / total_pixels;
            block.normalized_dark_ratio = static_cast<double>(normalized_dark_count) / total_pixels;
            block.normalized_midtone_ratio = static_cast<double>(normalized_midtone_count) / total_pixels;
            block.text_projection_score = row_projection_score(row_dark_counts, block.width);
            block.mean = sum / total_pixels;
            block.variance = sum_squares / total_pixels - block.mean * block.mean;
            block.block_class = classify_by_features(
                block.white_ratio,
                block.dark_ratio,
                block.nonwhite_ratio,
                block.midtone_ratio,
                block.normalized_white_ratio,
                block.normalized_dark_ratio,
                block.normalized_midtone_ratio,
                block.text_projection_score
            );

            result.blocks.push_back(block);
        }
    }

    return result;
}

void merge_image_blocks(
    BlockClassification& classification,
    const std::size_t min_region_blocks,
    const std::size_t min_neighbor_count
) {
    if (
        classification.grid_width == 0 ||
        classification.grid_height == 0 ||
        classification.blocks.size() != classification.grid_width * classification.grid_height
    ) {
        throw std::invalid_argument("invalid block classification grid");
    }

    classification.min_image_region_blocks = min_region_blocks;
    classification.min_image_neighbors = min_neighbor_count;
    classification.image_regions.clear();

    for (BlockInfo& block : classification.blocks) {
        block.image_region_id = -1;
        block.image_neighbor_count = 0;
    }

    for (std::size_t row = 0; row < classification.grid_height; ++row) {
        for (std::size_t column = 0; column < classification.grid_width; ++column) {
            const std::size_t index = grid_index(column, row, classification.grid_width);
            if (classification.blocks[index].block_class == BlockClass::image) {
                classification.blocks[index].image_neighbor_count =
                    image_neighbors(classification, column, row).size();
            }
        }
    }

    std::vector<bool> visited(classification.blocks.size(), false);
    int next_region_id = 0;

    for (std::size_t row = 0; row < classification.grid_height; ++row) {
        for (std::size_t column = 0; column < classification.grid_width; ++column) {
            const std::size_t start_index = grid_index(column, row, classification.grid_width);
            if (
                visited[start_index] ||
                classification.blocks[start_index].block_class != BlockClass::image
            ) {
                continue;
            }

            std::vector<std::size_t> component;
            std::deque<std::pair<std::size_t, std::size_t>> queue;
            visited[start_index] = true;
            queue.push_back({column, row});

            while (!queue.empty()) {
                const auto [current_column, current_row] = queue.front();
                queue.pop_front();

                const std::size_t current_index = grid_index(
                    current_column,
                    current_row,
                    classification.grid_width
                );
                component.push_back(current_index);

                for (const std::size_t neighbor_index : image_neighbors(classification, current_column, current_row)) {
                    if (!visited[neighbor_index]) {
                        visited[neighbor_index] = true;
                        queue.push_back({
                            neighbor_index % classification.grid_width,
                            neighbor_index / classification.grid_width
                        });
                    }
                }
            }

            bool has_supported_block = false;
            for (const std::size_t index : component) {
                if (classification.blocks[index].image_neighbor_count >= min_neighbor_count) {
                    has_supported_block = true;
                }
            }

            if (component.size() < min_region_blocks || !has_supported_block) {
                for (const std::size_t index : component) {
                    BlockInfo& block = classification.blocks[index];
                    block.block_class = fallback_class_for_removed_image_block(block);
                    block.image_region_id = -1;
                    block.image_neighbor_count = 0;
                }
                continue;
            }

            ImageRegionInfo region;
            region.id = next_region_id++;
            region.block_count = component.size();
            std::size_t min_x = std::numeric_limits<std::size_t>::max();
            std::size_t min_y = std::numeric_limits<std::size_t>::max();
            std::size_t max_x = 0;
            std::size_t max_y = 0;

            for (const std::size_t index : component) {
                BlockInfo& block = classification.blocks[index];
                block.image_region_id = region.id;
                min_x = std::min(min_x, block.x);
                min_y = std::min(min_y, block.y);
                max_x = std::max(max_x, block.x + block.width);
                max_y = std::max(max_y, block.y + block.height);
            }

            region.x = min_x;
            region.y = min_y;
            region.width = max_x - min_x;
            region.height = max_y - min_y;
            classification.image_regions.push_back(region);
        }
    }
}

RgbImage render_block_classification_overlay(
    const GrayImage& image,
    const BlockClassification& classification
) {
    if (
        image.empty() ||
        classification.image_width != image.width() ||
        classification.image_height != image.height()
    ) {
        throw std::invalid_argument("image and block classification dimensions must match");
    }

    RgbImage result;
    result.width = image.width();
    result.height = image.height();
    result.pixels.resize(image.size());

    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            const GrayImage::Pixel pixel = image.at(x, y);
            result.pixels[y * image.width() + x] = {pixel, pixel, pixel};
        }
    }

    for (const BlockInfo& block : classification.blocks) {
        if (block.block_class != BlockClass::image) {
            continue;
        }

        for (std::size_t dy = 0; dy < block.height; ++dy) {
            for (std::size_t dx = 0; dx < block.width; ++dx) {
                const std::size_t x = block.x + dx;
                const std::size_t y = block.y + dy;
                result.pixels[y * image.width() + x] = blend(image.at(x, y), {255, 0, 0}, overlay_alpha);
            }
        }
    }

    return result;
}

} // namespace dip::homework
