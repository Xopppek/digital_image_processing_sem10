#pragma once

#include "core/gray_image.hpp"
#include "homework/block_classifier.hpp"

#include <cstddef>

namespace dip::homework {

struct HalftonePipelineOptions {
    std::size_t block_size{56};
    std::size_t min_region_blocks{12};
    std::size_t min_neighbors{1};
    double rectangle_padding_fraction{0.20};
    std::size_t gaussian_kernel_size{11};
    double gaussian_sigma{1.8};
    double sharpen_amount{0.35};
};

struct HalftonePipelineResult {
    GrayImage blurred;
    GrayImage restored;
    BlockClassification classification;
    std::size_t processed_region_count{0};
};

[[nodiscard]] HalftonePipelineResult restore_halftone_scan(
    const GrayImage& image,
    const HalftonePipelineOptions& options
);

[[nodiscard]] RgbImage render_halftone_processing_overlay(
    const GrayImage& image,
    const BlockClassification& classification,
    const HalftonePipelineOptions& options
);

} // namespace dip::homework
