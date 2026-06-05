#pragma once

#include "core/gray_image.hpp"
#include "core/image_io.hpp"

#include <cstddef>
#include <vector>

namespace dip::homework {

enum class BlockClass {
    background,
    text,
    image
};

struct BlockInfo {
    std::size_t x{0};
    std::size_t y{0};
    std::size_t width{0};
    std::size_t height{0};
    BlockClass block_class{BlockClass::background};
    int image_region_id{-1};
    std::size_t image_neighbor_count{0};
    double white_ratio{0.0};
    double dark_ratio{0.0};
    double nonwhite_ratio{0.0};
    double midtone_ratio{0.0};
    double normalized_white_ratio{0.0};
    double normalized_dark_ratio{0.0};
    double normalized_midtone_ratio{0.0};
    double text_projection_score{0.0};
    double mean{0.0};
    double variance{0.0};
};

struct ImageRegionInfo {
    int id{-1};
    std::size_t block_count{0};
    std::size_t x{0};
    std::size_t y{0};
    std::size_t width{0};
    std::size_t height{0};
};

struct BlockClassification {
    std::size_t image_width{0};
    std::size_t image_height{0};
    std::size_t block_size{0};
    std::size_t grid_width{0};
    std::size_t grid_height{0};
    std::size_t min_image_region_blocks{0};
    std::size_t min_image_neighbors{0};
    std::vector<BlockInfo> blocks;
    std::vector<ImageRegionInfo> image_regions;
};

[[nodiscard]] const char* block_class_name(BlockClass block_class) noexcept;
[[nodiscard]] BlockClassification classify_blocks(const GrayImage& image, std::size_t block_size);
void merge_image_blocks(
    BlockClassification& classification,
    std::size_t min_region_blocks,
    std::size_t min_neighbor_count
);
[[nodiscard]] RgbImage render_block_classification_overlay(
    const GrayImage& image,
    const BlockClassification& classification
);

} // namespace dip::homework
