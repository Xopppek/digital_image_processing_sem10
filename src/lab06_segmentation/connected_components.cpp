#include "lab06_segmentation/connected_components.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dip::lab06 {
namespace {

std::size_t index_of(const std::size_t x, const std::size_t y, const std::size_t width) {
    return y * width + x;
}

RgbPixel label_to_color(const std::size_t label) {
    constexpr std::array<RgbPixel, 12> palette{{
        {230, 57, 70},
        {29, 53, 87},
        {42, 157, 143},
        {233, 196, 106},
        {244, 162, 97},
        {131, 56, 236},
        {0, 119, 182},
        {255, 183, 3},
        {106, 153, 78},
        {214, 40, 40},
        {58, 134, 255},
        {255, 0, 110},
    }};

    if (label == 0) {
        return {0, 0, 0};
    }

    return palette[(label - 1) % palette.size()];
}

} // namespace

ConnectedComponentsResult label_4_connected_components(const GrayImage& binary_image) {
    if (binary_image.empty()) {
        throw std::invalid_argument("binary image must not be empty");
    }

    ConnectedComponentsResult result;
    result.width = binary_image.width();
    result.height = binary_image.height();
    result.labels.assign(binary_image.size(), 0);

    std::vector<std::size_t> stack;
    stack.reserve(binary_image.size());
    std::size_t next_label = 1;

    for (std::size_t y = 0; y < binary_image.height(); ++y) {
        for (std::size_t x = 0; x < binary_image.width(); ++x) {
            const std::size_t start_index = index_of(x, y, binary_image.width());
            if (binary_image.at(x, y) == 0 || result.labels[start_index] != 0) {
                continue;
            }

            ConnectedComponent component;
            component.label = next_label;
            component.min_x = x;
            component.max_x = x;
            component.min_y = y;
            component.max_y = y;

            result.labels[start_index] = next_label;
            stack.clear();
            stack.push_back(start_index);

            while (!stack.empty()) {
                const std::size_t current = stack.back();
                stack.pop_back();

                const std::size_t current_x = current % binary_image.width();
                const std::size_t current_y = current / binary_image.width();
                const double x_value = static_cast<double>(current_x);
                const double y_value = static_cast<double>(current_y);

                ++component.area;
                component.m00 += 1.0;
                component.m10 += x_value;
                component.m01 += y_value;
                component.min_x = std::min(component.min_x, current_x);
                component.max_x = std::max(component.max_x, current_x);
                component.min_y = std::min(component.min_y, current_y);
                component.max_y = std::max(component.max_y, current_y);

                const std::array<std::pair<std::size_t, std::size_t>, 4> neighbors{{
                    {current_x > 0 ? current_x - 1 : current_x, current_y},
                    {current_x + 1 < binary_image.width() ? current_x + 1 : current_x, current_y},
                    {current_x, current_y > 0 ? current_y - 1 : current_y},
                    {current_x, current_y + 1 < binary_image.height() ? current_y + 1 : current_y},
                }};

                for (const auto& [neighbor_x, neighbor_y] : neighbors) {
                    if (neighbor_x == current_x && neighbor_y == current_y) {
                        continue;
                    }

                    const std::size_t neighbor_index = index_of(neighbor_x, neighbor_y, binary_image.width());
                    if (binary_image.at(neighbor_x, neighbor_y) == 0 || result.labels[neighbor_index] != 0) {
                        continue;
                    }

                    result.labels[neighbor_index] = next_label;
                    stack.push_back(neighbor_index);
                }
            }

            double m20 = 0.0;
            double m02 = 0.0;
            double m11 = 0.0;
            for (std::size_t point_y = component.min_y; point_y <= component.max_y; ++point_y) {
                for (std::size_t point_x = component.min_x; point_x <= component.max_x; ++point_x) {
                    const std::size_t point_index = index_of(point_x, point_y, binary_image.width());
                    if (result.labels[point_index] != next_label) {
                        continue;
                    }

                    const double x_value = static_cast<double>(point_x);
                    const double y_value = static_cast<double>(point_y);
                    m20 += x_value * x_value;
                    m02 += y_value * y_value;
                    m11 += x_value * y_value;
                }
            }

            component.centroid_x = component.m10 / component.m00;
            component.centroid_y = component.m01 / component.m00;
            component.mu20 = m20 - component.m10 * component.m10 / component.m00;
            component.mu02 = m02 - component.m01 * component.m01 / component.m00;
            component.mu11 = m11 - component.m10 * component.m01 / component.m00;
            result.components.push_back(component);
            ++next_label;
        }
    }

    return result;
}

GrayImage render_component_labels(const ConnectedComponentsResult& result) {
    if (result.width == 0 || result.height == 0 || result.labels.size() != result.width * result.height) {
        throw std::invalid_argument("invalid connected components result");
    }

    GrayImage image(result.width, result.height);
    for (std::size_t y = 0; y < result.height; ++y) {
        for (std::size_t x = 0; x < result.width; ++x) {
            const std::size_t label = result.labels[index_of(x, y, result.width)];
            image.at(x, y) = label == 0 ? 0 : static_cast<GrayImage::Pixel>((label * 37) % 255 + 1);
        }
    }

    return image;
}

RgbImage render_component_labels_color(const ConnectedComponentsResult& result) {
    if (result.width == 0 || result.height == 0 || result.labels.size() != result.width * result.height) {
        throw std::invalid_argument("invalid connected components result");
    }

    RgbImage image;
    image.width = result.width;
    image.height = result.height;
    image.pixels.resize(result.width * result.height);

    for (std::size_t y = 0; y < result.height; ++y) {
        for (std::size_t x = 0; x < result.width; ++x) {
            image.pixels[index_of(x, y, result.width)] = label_to_color(result.labels[index_of(x, y, result.width)]);
        }
    }

    return image;
}

std::vector<CircleLikeComponent> find_large_circle_like_components(
    const ConnectedComponentsResult& result,
    const std::size_t min_area,
    const double max_aspect_deviation,
    const double min_fill_ratio,
    const double max_fill_ratio,
    const double max_moment_deviation
) {
    std::vector<CircleLikeComponent> selected;

    for (const ConnectedComponent& component : result.components) {
        if (component.area <= min_area) {
            continue;
        }

        const double bounding_width = static_cast<double>(component.max_x - component.min_x + 1);
        const double bounding_height = static_cast<double>(component.max_y - component.min_y + 1);
        const double larger_side = std::max(bounding_width, bounding_height);
        const double aspect_deviation = std::abs(bounding_width - bounding_height) / larger_side;
        const double fill_ratio = static_cast<double>(component.area) / (bounding_width * bounding_height);
        const double moment_sum = component.mu20 + component.mu02;
        if (moment_sum <= 0.0) {
            continue;
        }

        const double moment_deviation = std::abs(component.mu20 - component.mu02) / moment_sum;
        if (aspect_deviation > max_aspect_deviation ||
            fill_ratio < min_fill_ratio ||
            fill_ratio > max_fill_ratio ||
            moment_deviation > max_moment_deviation) {
            continue;
        }

        selected.push_back({
            component,
            bounding_width,
            bounding_height,
            aspect_deviation,
            fill_ratio,
            moment_deviation,
        });
    }

    return selected;
}

RgbImage render_selected_components_color(
    const ConnectedComponentsResult& result,
    const std::vector<CircleLikeComponent>& selected_components
) {
    if (result.width == 0 || result.height == 0 || result.labels.size() != result.width * result.height) {
        throw std::invalid_argument("invalid connected components result");
    }

    std::vector<std::uint8_t> selected_labels(result.components.size() + 1, 0);
    for (const CircleLikeComponent& selected : selected_components) {
        if (selected.component.label < selected_labels.size()) {
            selected_labels[selected.component.label] = 1;
        }
    }

    RgbImage image;
    image.width = result.width;
    image.height = result.height;
    image.pixels.resize(result.width * result.height);

    for (std::size_t y = 0; y < result.height; ++y) {
        for (std::size_t x = 0; x < result.width; ++x) {
            const std::size_t label = result.labels[index_of(x, y, result.width)];
            const std::size_t pixel_index = index_of(x, y, result.width);
            if (label == 0) {
                image.pixels[pixel_index] = {0, 0, 0};
            } else if (label < selected_labels.size() && selected_labels[label] != 0) {
                image.pixels[pixel_index] = label_to_color(label);
            } else {
                image.pixels[pixel_index] = {55, 55, 55};
            }
        }
    }

    return image;
}

} // namespace dip::lab06
