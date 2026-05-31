#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dip {

class GrayImage {
public:
    using Pixel = std::uint8_t;

    GrayImage() = default;
    GrayImage(std::size_t width, std::size_t height);
    GrayImage(std::size_t width, std::size_t height, std::vector<Pixel> pixels);

    [[nodiscard]] std::size_t width() const noexcept;
    [[nodiscard]] std::size_t height() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] const std::vector<Pixel>& pixels() const noexcept;
    [[nodiscard]] std::vector<Pixel>& pixels() noexcept;

    [[nodiscard]] const Pixel* data() const noexcept;
    [[nodiscard]] Pixel* data() noexcept;

    [[nodiscard]] Pixel at(std::size_t x, std::size_t y) const;
    Pixel& at(std::size_t x, std::size_t y);

private:
    std::size_t width_{0};
    std::size_t height_{0};
    std::vector<Pixel> pixels_;

    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y) const;
};

} // namespace dip
