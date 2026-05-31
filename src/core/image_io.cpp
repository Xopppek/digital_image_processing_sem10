#include "core/image_io.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace dip {

GrayImage read_gray_image(const std::string& path) {
    const cv::Mat matrix = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (matrix.empty()) {
        throw std::runtime_error("failed to read image: " + path);
    }

    std::vector<GrayImage::Pixel> pixels;
    pixels.reserve(static_cast<std::size_t>(matrix.rows) * static_cast<std::size_t>(matrix.cols));

    for (int y = 0; y < matrix.rows; ++y) {
        const auto* row = matrix.ptr<GrayImage::Pixel>(y);
        pixels.insert(pixels.end(), row, row + matrix.cols);
    }

    return GrayImage(
        static_cast<std::size_t>(matrix.cols),
        static_cast<std::size_t>(matrix.rows),
        std::move(pixels)
    );
}

void write_gray_image(const std::string& path, const GrayImage& image) {
    if (image.empty()) {
        throw std::invalid_argument("cannot write an empty image");
    }

    cv::Mat matrix(
        static_cast<int>(image.height()),
        static_cast<int>(image.width()),
        CV_8UC1
    );
    std::copy(image.pixels().begin(), image.pixels().end(), matrix.data);

    if (!cv::imwrite(path, matrix)) {
        throw std::runtime_error("failed to write image: " + path);
    }
}

} // namespace dip
