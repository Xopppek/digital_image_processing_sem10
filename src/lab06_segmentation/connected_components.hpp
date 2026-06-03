#pragma once

#include "core/gray_image.hpp"
#include "core/image_io.hpp"

#include <cstddef>
#include <vector>

namespace dip::lab06 {

struct ConnectedComponent {
    std::size_t label{0};
    std::size_t area{0};
    std::size_t min_x{0};
    std::size_t min_y{0};
    std::size_t max_x{0};
    std::size_t max_y{0};
    double m00{0.0};
    double m10{0.0};
    double m01{0.0};
    double centroid_x{0.0};
    double centroid_y{0.0};
    double mu20{0.0};
    double mu02{0.0};
    double mu11{0.0};
};

struct ConnectedComponentsResult {
    std::size_t width{0};
    std::size_t height{0};
    std::vector<std::size_t> labels;
    std::vector<ConnectedComponent> components;
};

struct CircleLikeComponent {
    ConnectedComponent component;
    double bounding_width{0.0};
    double bounding_height{0.0};
    double aspect_deviation{0.0};
    double fill_ratio{0.0};
    double moment_deviation{0.0};
};

[[nodiscard]] ConnectedComponentsResult label_4_connected_components(const GrayImage& binary_image);
[[nodiscard]] GrayImage render_component_labels(const ConnectedComponentsResult& result);
[[nodiscard]] RgbImage render_component_labels_color(const ConnectedComponentsResult& result);
[[nodiscard]] std::vector<CircleLikeComponent> find_large_circle_like_components(
    const ConnectedComponentsResult& result,
    std::size_t min_area,
    double max_aspect_deviation,
    double min_fill_ratio,
    double max_fill_ratio,
    double max_moment_deviation
);
[[nodiscard]] RgbImage render_selected_components_color(
    const ConnectedComponentsResult& result,
    const std::vector<CircleLikeComponent>& selected_components
);

} // namespace dip::lab06
