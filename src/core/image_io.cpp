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

void write_rgb_image(const std::string& path, const RgbImage& image) {
    if (image.width == 0 || image.height == 0 || image.pixels.size() != image.width * image.height) {
        throw std::invalid_argument("cannot write an empty or invalid RGB image");
    }

    cv::Mat matrix(
        static_cast<int>(image.height),
        static_cast<int>(image.width),
        CV_8UC3
    );

    for (std::size_t y = 0; y < image.height; ++y) {
        auto* row = matrix.ptr<std::uint8_t>(static_cast<int>(y));
        for (std::size_t x = 0; x < image.width; ++x) {
            const RgbPixel& pixel = image.pixels[y * image.width + x];
            row[x * 3] = pixel.blue;
            row[x * 3 + 1] = pixel.green;
            row[x * 3 + 2] = pixel.red;
        }
    }

    if (!cv::imwrite(path, matrix)) {
        throw std::runtime_error("failed to write image: " + path);
    }
}

} // namespace dip
