#include "lab05_nonlinear/morphology.hpp"

namespace dip::lab05 {

GrayImage erosion_valid(const GrayImage& image, const Aperture& structuring_element) {
    return rank_filter_valid(image, structuring_element, 0);
}

GrayImage dilation_valid(const GrayImage& image, const Aperture& structuring_element) {
    const std::size_t active_size = active_aperture_size(structuring_element);
    return rank_filter_valid(image, structuring_element, active_size - 1);
}

GrayImage opening_valid(const GrayImage& image, const Aperture& structuring_element) {
    return dilation_valid(erosion_valid(image, structuring_element), structuring_element);
}

GrayImage closing_valid(const GrayImage& image, const Aperture& structuring_element) {
    return erosion_valid(dilation_valid(image, structuring_element), structuring_element);
}

} // namespace dip::lab05
