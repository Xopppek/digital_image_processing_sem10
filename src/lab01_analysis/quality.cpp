#include "lab01_analysis/quality.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace dip::lab01 {
namespace {

void require_same_size(const GrayImage& original, const GrayImage& distorted) {
    if (original.empty() || distorted.empty()) {
        throw std::invalid_argument("MSE and PSNR require non-empty images");
    }

    if (original.width() != distorted.width() || original.height() != distorted.height()) {
        throw std::invalid_argument("MSE and PSNR require images of the same size");
    }
}

} // namespace

double mean_squared_error(const GrayImage& original, const GrayImage& distorted) {
    require_same_size(original, distorted);

    double squared_error_sum = 0.0;

    for (std::size_t i = 0; i < original.size(); ++i) {
        const double difference = static_cast<double>(original.pixels()[i]) -
                                  static_cast<double>(distorted.pixels()[i]);
        squared_error_sum += difference * difference;
    }

    return squared_error_sum / static_cast<double>(original.size());
}

double peak_signal_to_noise_ratio(const GrayImage& original, const GrayImage& distorted) {
    const double mse = mean_squared_error(original, distorted);

    if (mse == 0.0) {
        return std::numeric_limits<double>::infinity();
    }

    constexpr double max_pixel_value = 255.0;
    return 20.0 * std::log10(max_pixel_value / std::sqrt(mse));
}

} // namespace dip::lab01
