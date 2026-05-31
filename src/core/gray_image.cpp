#include "core/gray_image.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace dip {

GrayImage::GrayImage(const std::size_t width, const std::size_t height)
    : width_(width),
      height_(height),
      pixels_(width * height, Pixel{0}) {}

GrayImage::GrayImage(const std::size_t width, const std::size_t height, std::vector<Pixel> pixels)
    : width_(width),
      height_(height),
      pixels_(std::move(pixels)) {
    if (pixels_.size() != width_ * height_) {
        throw std::invalid_argument("GrayImage pixel count does not match dimensions");
    }
}

std::size_t GrayImage::width() const noexcept {
    return width_;
}

std::size_t GrayImage::height() const noexcept {
    return height_;
}

bool GrayImage::empty() const noexcept {
    return pixels_.empty();
}

std::size_t GrayImage::size() const noexcept {
    return pixels_.size();
}

const std::vector<GrayImage::Pixel>& GrayImage::pixels() const noexcept {
    return pixels_;
}

std::vector<GrayImage::Pixel>& GrayImage::pixels() noexcept {
    return pixels_;
}

const GrayImage::Pixel* GrayImage::data() const noexcept {
    return pixels_.data();
}

GrayImage::Pixel* GrayImage::data() noexcept {
    return pixels_.data();
}

GrayImage::Pixel GrayImage::at(const std::size_t x, const std::size_t y) const {
    return pixels_.at(index(x, y));
}

GrayImage::Pixel& GrayImage::at(const std::size_t x, const std::size_t y) {
    return pixels_.at(index(x, y));
}

std::size_t GrayImage::index(const std::size_t x, const std::size_t y) const {
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("GrayImage coordinates are outside image bounds");
    }

    return y * width_ + x;
}

} // namespace dip
