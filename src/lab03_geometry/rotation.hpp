#pragma once

#include "core/gray_image.hpp"

namespace dip::lab03 {

enum class InterpolationMethod {
    nearest,
    bilinear,
    bicubic,
};

[[nodiscard]] const char* interpolation_method_name(InterpolationMethod method) noexcept;

[[nodiscard]] GrayImage rotate_image(
    const GrayImage& image,
    double angle_degrees,
    InterpolationMethod method,
    GrayImage::Pixel background = 0
);

[[nodiscard]] GrayImage rotate_nearest(
    const GrayImage& image,
    double angle_degrees,
    GrayImage::Pixel background = 0
);

[[nodiscard]] GrayImage rotate_bilinear(
    const GrayImage& image,
    double angle_degrees,
    GrayImage::Pixel background = 0
);

[[nodiscard]] GrayImage rotate_bicubic(
    const GrayImage& image,
    double angle_degrees,
    GrayImage::Pixel background = 0
);

} // namespace dip::lab03
