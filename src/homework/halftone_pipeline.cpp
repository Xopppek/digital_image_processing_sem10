#include "homework/halftone_pipeline.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace dip::homework {
namespace {

constexpr double overlay_alpha = 0.35;

GrayImage::Pixel clamp_to_pixel(const double value) {
    return static_cast<GrayImage::Pixel>(
        std::clamp(static_cast<int>(std::round(value)), 0, 255)
    );
}

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

void validate_filter_options(const HalftonePipelineOptions& options) {
    if (options.gaussian_kernel_size == 0 || options.gaussian_kernel_size % 2 == 0) {
        throw std::invalid_argument("Gaussian kernel size must be an odd positive value");
    }

    if (options.gaussian_sigma <= 0.0) {
        throw std::invalid_argument("Gaussian sigma must be positive");
    }

    if (options.sharpen_amount < 0.0) {
        throw std::invalid_argument("sharpen amount must not be negative");
    }

    if (options.rectangle_padding_fraction < 0.0) {
        throw std::invalid_argument("rectangle padding fraction must not be negative");
    }
}

bool is_text_like_histogram_block(const BlockInfo& block) {
    return
        (
            block.white_ratio >= 0.65 &&
            block.nonwhite_ratio >= 0.015 &&
            block.nonwhite_ratio <= 0.35 &&
            block.midtone_ratio <= 0.25 &&
            block.dark_ratio <= 0.35
        ) ||
        (
            block.normalized_white_ratio >= 0.45 &&
            block.normalized_dark_ratio >= 0.02 &&
            block.normalized_dark_ratio <= 0.45 &&
            block.normalized_midtone_ratio <= 0.45 &&
            block.text_projection_score >= 0.22
        );
}

bool is_protected_text_block(const BlockInfo& block) {
    return block.block_class == BlockClass::text || is_text_like_histogram_block(block);
}

bool ranges_overlap(
    const std::size_t first_begin,
    const std::size_t first_end,
    const std::size_t second_begin,
    const std::size_t second_end
) {
    return first_begin < second_end && second_begin < first_end;
}

ImageRegionInfo processing_region(
    const ImageRegionInfo& region,
    const BlockClassification& classification,
    const HalftonePipelineOptions& options
) {
    ImageRegionInfo result;
    result.id = region.id;

    std::size_t min_x = classification.image_width;
    std::size_t min_y = classification.image_height;
    std::size_t max_x = 0;
    std::size_t max_y = 0;

    for (const BlockInfo& block : classification.blocks) {
        if (
            block.image_region_id != region.id ||
            block.block_class != BlockClass::image ||
            is_protected_text_block(block)
        ) {
            continue;
        }

        ++result.block_count;
        min_x = std::min(min_x, block.x);
        min_y = std::min(min_y, block.y);
        max_x = std::max(max_x, block.x + block.width);
        max_y = std::max(max_y, block.y + block.height);
    }

    if (result.block_count == 0) {
        return result;
    }

    const std::size_t padding = static_cast<std::size_t>(
        std::round(static_cast<double>(classification.block_size) * options.rectangle_padding_fraction)
    );

    std::size_t left = min_x > padding ? min_x - padding : 0;
    std::size_t top = min_y > padding ? min_y - padding : 0;
    std::size_t right = std::min(classification.image_width, max_x + padding);
    std::size_t bottom = std::min(classification.image_height, max_y + padding);

    for (const BlockInfo& block : classification.blocks) {
        if (!is_protected_text_block(block)) {
            continue;
        }

        const std::size_t block_right = block.x + block.width;
        const std::size_t block_bottom = block.y + block.height;

        if (ranges_overlap(min_x, max_x, block.x, block_right)) {
            if (block_bottom <= min_y && block_bottom > top) {
                top = block_bottom;
            }
            if (block.y >= max_y && block.y < bottom) {
                if (block.height < classification.block_size) {
                    bottom = block.y > padding ? block.y - padding : block.y;
                } else {
                    bottom = block.y;
                }
            }
        }

        if (ranges_overlap(min_y, max_y, block.y, block_bottom)) {
            if (block_right <= min_x && block_right > left) {
                left = block_right;
            }
            if (block.x >= max_x && block.x < right) {
                right = block.x;
            }
        }
    }

    result.x = left;
    result.y = top;
    result.width = right - result.x;
    result.height = bottom - result.y;
    return result;
}

std::vector<bool> build_image_mask(
    const BlockClassification& classification,
    const HalftonePipelineOptions& options
) {
    std::vector<bool> mask(classification.image_width * classification.image_height, false);

    for (const ImageRegionInfo& region : classification.image_regions) {
        const ImageRegionInfo area = processing_region(region, classification, options);
        for (std::size_t dy = 0; dy < area.height; ++dy) {
            for (std::size_t dx = 0; dx < area.width; ++dx) {
                const std::size_t x = area.x + dx;
                const std::size_t y = area.y + dy;
                mask[y * classification.image_width + x] = true;
            }
        }
    }

    return mask;
}

std::vector<double> gaussian_kernel(const std::size_t size, const double sigma) {
    const int radius = static_cast<int>(size / 2);
    std::vector<double> kernel(size * size, 0.0);
    double sum = 0.0;

    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            const double distance_square = static_cast<double>(x * x + y * y);
            const double value = std::exp(-distance_square / (2.0 * sigma * sigma));
            kernel[static_cast<std::size_t>(y + radius) * size + static_cast<std::size_t>(x + radius)] = value;
            sum += value;
        }
    }

    for (double& value : kernel) {
        value /= sum;
    }

    return kernel;
}

double gaussian_sample(
    const GrayImage& image,
    const std::vector<bool>& image_mask,
    const std::vector<double>& kernel,
    const std::size_t kernel_size,
    const std::size_t x,
    const std::size_t y
) {
    const int radius = static_cast<int>(kernel_size / 2);
    double weighted_sum = 0.0;
    double weight_sum = 0.0;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int sample_x = static_cast<int>(x) + dx;
            const int sample_y = static_cast<int>(y) + dy;
            if (
                sample_x < 0 ||
                sample_y < 0 ||
                sample_x >= static_cast<int>(image.width()) ||
                sample_y >= static_cast<int>(image.height())
            ) {
                continue;
            }

            const std::size_t sx = static_cast<std::size_t>(sample_x);
            const std::size_t sy = static_cast<std::size_t>(sample_y);
            if (!image_mask[sy * image.width() + sx]) {
                continue;
            }

            const std::size_t kernel_x = static_cast<std::size_t>(dx + radius);
            const std::size_t kernel_y = static_cast<std::size_t>(dy + radius);
            const double weight = kernel[kernel_y * kernel_size + kernel_x];
            weighted_sum += static_cast<double>(image.at(sx, sy)) * weight;
            weight_sum += weight;
        }
    }

    if (weight_sum == 0.0) {
        return static_cast<double>(image.at(x, y));
    }

    return weighted_sum / weight_sum;
}

GrayImage gaussian_blur_masked(
    const GrayImage& source,
    const std::vector<bool>& image_mask,
    const std::vector<double>& kernel,
    const std::size_t kernel_size
) {
    GrayImage result = source;

    for (std::size_t y = 0; y < source.height(); ++y) {
        for (std::size_t x = 0; x < source.width(); ++x) {
            if (!image_mask[y * source.width() + x]) {
                continue;
            }

            result.at(x, y) = clamp_to_pixel(gaussian_sample(source, image_mask, kernel, kernel_size, x, y));
        }
    }

    return result;
}

double masked_pixel_or_center(
    const GrayImage& image,
    const std::vector<bool>& image_mask,
    const std::size_t center_x,
    const std::size_t center_y,
    const int sample_x,
    const int sample_y
) {
    if (
        sample_x < 0 ||
        sample_y < 0 ||
        sample_x >= static_cast<int>(image.width()) ||
        sample_y >= static_cast<int>(image.height())
    ) {
        return static_cast<double>(image.at(center_x, center_y));
    }

    const std::size_t x = static_cast<std::size_t>(sample_x);
    const std::size_t y = static_cast<std::size_t>(sample_y);
    if (!image_mask[y * image.width() + x]) {
        return static_cast<double>(image.at(center_x, center_y));
    }

    return static_cast<double>(image.at(x, y));
}

GrayImage laplacian_sharpen_masked(
    const GrayImage& image,
    const std::vector<bool>& image_mask,
    const double amount
) {
    GrayImage result = image;

    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            if (!image_mask[y * image.width() + x]) {
                continue;
            }

            const double center = static_cast<double>(image.at(x, y));
            const double left = masked_pixel_or_center(
                image, image_mask, x, y, static_cast<int>(x) - 1, static_cast<int>(y)
            );
            const double right = masked_pixel_or_center(
                image, image_mask, x, y, static_cast<int>(x) + 1, static_cast<int>(y)
            );
            const double top = masked_pixel_or_center(
                image, image_mask, x, y, static_cast<int>(x), static_cast<int>(y) - 1
            );
            const double bottom = masked_pixel_or_center(
                image, image_mask, x, y, static_cast<int>(x), static_cast<int>(y) + 1
            );

            const double laplacian = 4.0 * center - left - right - top - bottom;
            result.at(x, y) = clamp_to_pixel(center + amount * laplacian);
        }
    }

    return result;
}

} // namespace

HalftonePipelineResult restore_halftone_scan(
    const GrayImage& image,
    const HalftonePipelineOptions& options
) {
    if (image.empty()) {
        throw std::invalid_argument("halftone pipeline requires a non-empty image");
    }

    validate_filter_options(options);

    HalftonePipelineResult result;
    result.classification = classify_blocks(image, options.block_size);
    merge_image_blocks(result.classification, options.min_region_blocks, options.min_neighbors);
    result.processed_region_count = result.classification.image_regions.size();

    const std::vector<bool> image_mask = build_image_mask(result.classification, options);
    const std::vector<double> kernel = gaussian_kernel(options.gaussian_kernel_size, options.gaussian_sigma);
    result.blurred = gaussian_blur_masked(image, image_mask, kernel, options.gaussian_kernel_size);
    result.restored = laplacian_sharpen_masked(result.blurred, image_mask, options.sharpen_amount);

    return result;
}

RgbImage render_halftone_processing_overlay(
    const GrayImage& image,
    const BlockClassification& classification,
    const HalftonePipelineOptions& options
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

    const std::vector<bool> mask = build_image_mask(classification, options);
    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            if (!mask[y * image.width() + x]) {
                continue;
            }

            result.pixels[y * image.width() + x] = blend(image.at(x, y), {255, 0, 0}, overlay_alpha);
        }
    }

    return result;
}

} // namespace dip::homework
