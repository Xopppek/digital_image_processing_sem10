#pragma once

#include "core/gray_image.hpp"
#include "lab01_analysis/histogram.hpp"

namespace dip::lab01 {

struct Quartiles {
    double q1{0.0};
    double q2{0.0};
    double q3{0.0};
};

struct ImageStatistics {
    Histogram histogram{};
    double mean{0.0};
    double variance{0.0};
    Quartiles quartiles{};
    double entropy{0.0};
    double energy{0.0};
    double skewness{0.0};
    double kurtosis{0.0};
};

[[nodiscard]] double mean(const Histogram& histogram);
[[nodiscard]] double mean(const GrayImage& image);

[[nodiscard]] double variance(const Histogram& histogram);
[[nodiscard]] double variance(const GrayImage& image);

[[nodiscard]] Quartiles quartiles(const Histogram& histogram);
[[nodiscard]] Quartiles quartiles(const GrayImage& image);

[[nodiscard]] double entropy(const Histogram& histogram);
[[nodiscard]] double entropy(const GrayImage& image);

[[nodiscard]] double energy(const Histogram& histogram);
[[nodiscard]] double energy(const GrayImage& image);

[[nodiscard]] double skewness(const Histogram& histogram, double mean_value, double variance_value);
[[nodiscard]] double skewness(const GrayImage& image);

[[nodiscard]] double kurtosis(const Histogram& histogram, double mean_value, double variance_value);
[[nodiscard]] double kurtosis(const GrayImage& image);

[[nodiscard]] ImageStatistics calculate_statistics(const GrayImage& image);

} // namespace dip::lab01
