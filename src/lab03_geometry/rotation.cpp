#include "lab03_geometry/rotation.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace dip::lab03 {
namespace {

constexpr double pi = 3.141592653589793238462643383279502884;

struct Bounds {
    double min_x{0.0};
    double max_x{0.0};
    double min_y{0.0};
    double max_y{0.0};
};

struct Point {
    double x{0.0};
    double y{0.0};
};

struct HomogeneousPoint {
    double x{0.0};
    double y{0.0};
    double w{1.0};
};

struct Matrix3x3 {
    std::array<std::array<double, 3>, 3> value{};
};

HomogeneousPoint transform_point(const HomogeneousPoint point, const Matrix3x3& transform) {
    return {
        point.x * transform.value[0][0] +
            point.y * transform.value[1][0] +
            point.w * transform.value[2][0],
        point.x * transform.value[0][1] +
            point.y * transform.value[1][1] +
            point.w * transform.value[2][1],
        point.x * transform.value[0][2] +
            point.y * transform.value[1][2] +
            point.w * transform.value[2][2],
    };
}

Point to_point(const HomogeneousPoint point) {
    if (point.w == 0.0) {
        return {point.x, point.y};
    }

    return {
        point.x / point.w,
        point.y / point.w,
    };
}

Matrix3x3 rotation_matrix(const double angle_radians) {
    const double cos_angle = std::cos(angle_radians);
    const double sin_angle = std::sin(angle_radians);

    Matrix3x3 transform;
    transform.value[0][0] = cos_angle;
    transform.value[0][1] = -sin_angle;
    transform.value[0][2] = 0.0;
    transform.value[1][0] = sin_angle;
    transform.value[1][1] = cos_angle;
    transform.value[1][2] = 0.0;
    transform.value[2][0] = 0.0;
    transform.value[2][1] = 0.0;
    transform.value[2][2] = 1.0;
    return transform;
}

Matrix3x3 inverse_rotation_matrix(const double angle_radians) {
    return rotation_matrix(-angle_radians);
}

Bounds rotated_bounds(const std::size_t width, const std::size_t height, const Matrix3x3& transform) {
    const double half_width = static_cast<double>(width) / 2.0;
    const double half_height = static_cast<double>(height) / 2.0;

    const std::array<HomogeneousPoint, 4> corners{{
        {-half_width, -half_height, 1.0},
        { half_width, -half_height, 1.0},
        {-half_width,  half_height, 1.0},
        { half_width,  half_height, 1.0},
    }};

    const Point first = to_point(transform_point(corners.front(), transform));
    Bounds bounds{first.x, first.x, first.y, first.y};

    for (const HomogeneousPoint corner : corners) {
        const Point rotated = to_point(transform_point(corner, transform));
        bounds.min_x = std::min(bounds.min_x, rotated.x);
        bounds.max_x = std::max(bounds.max_x, rotated.x);
        bounds.min_y = std::min(bounds.min_y, rotated.y);
        bounds.max_y = std::max(bounds.max_y, rotated.y);
    }

    return bounds;
}

GrayImage::Pixel pixel_or_background(
    const GrayImage& image,
    const int x,
    const int y,
    const GrayImage::Pixel background
) {
    if (
        x < 0 ||
        y < 0 ||
        x >= static_cast<int>(image.width()) ||
        y >= static_cast<int>(image.height())
    ) {
        return background;
    }

    return image.at(static_cast<std::size_t>(x), static_cast<std::size_t>(y));
}

GrayImage::Pixel clamp_to_pixel(const double value) {
    const auto rounded = static_cast<int>(std::round(value));
    return static_cast<GrayImage::Pixel>(std::clamp(rounded, 0, 255));
}

std::size_t ceil_to_size(const double value) {
    constexpr double epsilon = 1.0e-9;
    return static_cast<std::size_t>(std::max(1.0, std::ceil(value - epsilon)));
}

GrayImage::Pixel sample_nearest(
    const GrayImage& image,
    const double x,
    const double y,
    const GrayImage::Pixel background
) {
    const int nearest_x = static_cast<int>(std::floor(x + 0.5));
    const int nearest_y = static_cast<int>(std::floor(y + 0.5));
    return pixel_or_background(image, nearest_x, nearest_y, background);
}

GrayImage::Pixel sample_bilinear(
    const GrayImage& image,
    const double x,
    const double y,
    const GrayImage::Pixel background
) {
    if (
        x < 0.0 ||
        y < 0.0 ||
        x > static_cast<double>(image.width() - 1) ||
        y > static_cast<double>(image.height() - 1)
    ) {
        return background;
    }

    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;

    const double dx = x - static_cast<double>(x0);
    const double dy = y - static_cast<double>(y0);

    const double top_left = pixel_or_background(image, x0, y0, background);
    const double top_right = pixel_or_background(image, x1, y0, background);
    const double bottom_left = pixel_or_background(image, x0, y1, background);
    const double bottom_right = pixel_or_background(image, x1, y1, background);

    const double top = top_left * (1.0 - dx) + top_right * dx;
    const double bottom = bottom_left * (1.0 - dx) + bottom_right * dx;
    return clamp_to_pixel(top * (1.0 - dy) + bottom * dy);
}

double cubic_interpolate(
    const double p0,
    const double p1,
    const double p2,
    const double p3,
    const double t
) {
    return p1 + 0.5 * t * (
        p2 - p0 +
        t * (
            2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 +
            t * (3.0 * (p1 - p2) + p3 - p0)
        )
    );
}

GrayImage::Pixel sample_bicubic(
    const GrayImage& image,
    const double x,
    const double y,
    const GrayImage::Pixel background
) {
    if (
        x < 0.0 ||
        y < 0.0 ||
        x > static_cast<double>(image.width() - 1) ||
        y > static_cast<double>(image.height() - 1)
    ) {
        return background;
    }

    const int base_x = static_cast<int>(std::floor(x));
    const int base_y = static_cast<int>(std::floor(y));
    const double dx = x - static_cast<double>(base_x);
    const double dy = y - static_cast<double>(base_y);

    std::array<double, 4> rows{};
    for (int row_offset = -1; row_offset <= 2; ++row_offset) {
        std::array<double, 4> values{};
        for (int column_offset = -1; column_offset <= 2; ++column_offset) {
            values[static_cast<std::size_t>(column_offset + 1)] =
                pixel_or_background(image, base_x + column_offset, base_y + row_offset, background);
        }

        rows[static_cast<std::size_t>(row_offset + 1)] =
            cubic_interpolate(values[0], values[1], values[2], values[3], dx);
    }

    return clamp_to_pixel(cubic_interpolate(rows[0], rows[1], rows[2], rows[3], dy));
}

GrayImage::Pixel sample(
    const GrayImage& image,
    const double x,
    const double y,
    const InterpolationMethod method,
    const GrayImage::Pixel background
) {
    switch (method) {
        case InterpolationMethod::nearest:
            return sample_nearest(image, x, y, background);
        case InterpolationMethod::bilinear:
            return sample_bilinear(image, x, y, background);
        case InterpolationMethod::bicubic:
            return sample_bicubic(image, x, y, background);
    }

    return background;
}

}

const char* interpolation_method_name(const InterpolationMethod method) noexcept {
    switch (method) {
        case InterpolationMethod::nearest:
            return "nearest";
        case InterpolationMethod::bilinear:
            return "bilinear";
        case InterpolationMethod::bicubic:
            return "bicubic";
    }

    return "unknown";
}

GrayImage rotate_image(
    const GrayImage& image,
    const double angle_degrees,
    const InterpolationMethod method,
    const GrayImage::Pixel background
) {
    if (image.empty()) {
        throw std::invalid_argument("rotation requires a non-empty image");
    }

    if (!std::isfinite(angle_degrees)) {
        throw std::invalid_argument("rotation angle must be finite");
    }

    const double angle_radians = angle_degrees * pi / 180.0;
    const Matrix3x3 forward_transform = rotation_matrix(angle_radians);
    const Matrix3x3 inverse_transform = inverse_rotation_matrix(angle_radians);
    const Bounds bounds = rotated_bounds(image.width(), image.height(), forward_transform);

    const std::size_t output_width = ceil_to_size(bounds.max_x - bounds.min_x);
    const std::size_t output_height = ceil_to_size(bounds.max_y - bounds.min_y);

    GrayImage result(
        output_width,
        output_height,
        std::vector<GrayImage::Pixel>(output_width * output_height, background)
    );

    const double source_center_x = static_cast<double>(image.width()) / 2.0;
    const double source_center_y = static_cast<double>(image.height()) / 2.0;

    for (std::size_t row = 0; row < output_height; ++row) {
        const double rotated_y = bounds.min_y + static_cast<double>(row) + 0.5;

        for (std::size_t column = 0; column < output_width; ++column) {
            const double rotated_x = bounds.min_x + static_cast<double>(column) + 0.5;
            const Point source = to_point(transform_point({rotated_x, rotated_y, 1.0}, inverse_transform));
            const double source_x = source.x + source_center_x - 0.5;
            const double source_y = source.y + source_center_y - 0.5;

            result.at(column, row) = sample(image, source_x, source_y, method, background);
        }
    }

    return result;
}

GrayImage rotate_nearest(
    const GrayImage& image,
    const double angle_degrees,
    const GrayImage::Pixel background
) {
    return rotate_image(image, angle_degrees, InterpolationMethod::nearest, background);
}

GrayImage rotate_bilinear(
    const GrayImage& image,
    const double angle_degrees,
    const GrayImage::Pixel background
) {
    return rotate_image(image, angle_degrees, InterpolationMethod::bilinear, background);
}

GrayImage rotate_bicubic(
    const GrayImage& image,
    const double angle_degrees,
    const GrayImage::Pixel background
) {
    return rotate_image(image, angle_degrees, InterpolationMethod::bicubic, background);
}

} // namespace dip::lab03
