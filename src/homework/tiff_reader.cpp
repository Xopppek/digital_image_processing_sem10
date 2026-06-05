#include "homework/tiff_reader.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace dip::homework {
namespace {

enum class ByteOrder {
    intel,
    motorola
};

struct TiffEntry {
    std::uint16_t tag{0};
    std::uint16_t type{0};
    std::uint32_t count{0};
    std::uint32_t value_or_offset{0};
    std::size_t inline_value_offset{0};
};

struct TiffMetadata {
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::vector<std::uint16_t> bits_per_sample{8};
    std::uint16_t compression{1};
    std::uint16_t photometric_interpretation{1};
    std::uint16_t samples_per_pixel{1};
    std::uint16_t planar_configuration{1};
    std::vector<std::uint16_t> sample_format{1};
    std::uint32_t rows_per_strip{0};
    std::vector<std::uint32_t> strip_offsets;
    std::vector<std::uint32_t> strip_byte_counts;
};

constexpr std::uint16_t tag_image_width = 256;
constexpr std::uint16_t tag_image_length = 257;
constexpr std::uint16_t tag_bits_per_sample = 258;
constexpr std::uint16_t tag_compression = 259;
constexpr std::uint16_t tag_photometric_interpretation = 262;
constexpr std::uint16_t tag_strip_offsets = 273;
constexpr std::uint16_t tag_samples_per_pixel = 277;
constexpr std::uint16_t tag_rows_per_strip = 278;
constexpr std::uint16_t tag_strip_byte_counts = 279;
constexpr std::uint16_t tag_planar_configuration = 284;
constexpr std::uint16_t tag_sample_format = 339;

std::vector<std::uint8_t> read_file_bytes(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to open TIFF file: " + path);
    }

    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size < 0) {
        throw std::runtime_error("failed to determine TIFF file size: " + path);
    }

    input.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    if (!bytes.empty()) {
        input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    if (!input) {
        throw std::runtime_error("failed to read TIFF file: " + path);
    }

    return bytes;
}

void require_range(const std::vector<std::uint8_t>& bytes, const std::size_t offset, const std::size_t size) {
    if (offset > bytes.size() || size > bytes.size() - offset) {
        throw std::runtime_error("TIFF file is truncated or contains an invalid offset");
    }
}

std::uint16_t read_u16(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset,
    const ByteOrder byte_order
) {
    require_range(bytes, offset, 2);
    if (byte_order == ByteOrder::intel) {
        return static_cast<std::uint16_t>(bytes[offset] | (bytes[offset + 1] << 8));
    }

    return static_cast<std::uint16_t>((bytes[offset] << 8) | bytes[offset + 1]);
}

std::uint32_t read_u32(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset,
    const ByteOrder byte_order
) {
    require_range(bytes, offset, 4);
    if (byte_order == ByteOrder::intel) {
        return static_cast<std::uint32_t>(bytes[offset]) |
            (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
            (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
            (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
    }

    return (static_cast<std::uint32_t>(bytes[offset]) << 24) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
        static_cast<std::uint32_t>(bytes[offset + 3]);
}

std::size_t tiff_type_size(const std::uint16_t type) {
    switch (type) {
        case 1:
        case 2:
            return 1;
        case 3:
            return 2;
        case 4:
            return 4;
        case 5:
            return 8;
        default:
            throw std::runtime_error("unsupported TIFF field type: " + std::to_string(type));
    }
}

std::size_t checked_value_byte_count(const TiffEntry& entry) {
    const std::size_t element_size = tiff_type_size(entry.type);
    if (entry.count > std::numeric_limits<std::size_t>::max() / element_size) {
        throw std::runtime_error("TIFF field is too large");
    }

    return static_cast<std::size_t>(entry.count) * element_size;
}

std::size_t entry_value_offset(const TiffEntry& entry) {
    const std::size_t value_bytes = checked_value_byte_count(entry);
    if (value_bytes <= 4) {
        return entry.inline_value_offset;
    }

    return entry.value_or_offset;
}

std::uint32_t read_entry_value(
    const std::vector<std::uint8_t>& bytes,
    const TiffEntry& entry,
    const ByteOrder byte_order,
    const std::size_t index
) {
    if (index >= entry.count) {
        throw std::runtime_error("TIFF value index is out of range");
    }

    const std::size_t offset = entry_value_offset(entry) + index * tiff_type_size(entry.type);
    switch (entry.type) {
        case 1:
            require_range(bytes, offset, 1);
            return bytes[offset];
        case 3:
            return read_u16(bytes, offset, byte_order);
        case 4:
            return read_u32(bytes, offset, byte_order);
        default:
            throw std::runtime_error("unsupported TIFF numeric field type for tag: " + std::to_string(entry.tag));
    }
}

std::uint32_t read_single_value(
    const std::vector<std::uint8_t>& bytes,
    const TiffEntry& entry,
    const ByteOrder byte_order
) {
    if (entry.count != 1) {
        throw std::runtime_error("TIFF tag must contain exactly one value: " + std::to_string(entry.tag));
    }

    return read_entry_value(bytes, entry, byte_order, 0);
}

std::vector<std::uint32_t> read_value_vector(
    const std::vector<std::uint8_t>& bytes,
    const TiffEntry& entry,
    const ByteOrder byte_order
) {
    std::vector<std::uint32_t> values;
    values.reserve(entry.count);
    for (std::size_t index = 0; index < entry.count; ++index) {
        values.push_back(read_entry_value(bytes, entry, byte_order, index));
    }

    return values;
}

void apply_entry(
    const std::vector<std::uint8_t>& bytes,
    const TiffEntry& entry,
    const ByteOrder byte_order,
    TiffMetadata& metadata
) {
    switch (entry.tag) {
        case tag_image_width:
            metadata.width = read_single_value(bytes, entry, byte_order);
            break;
        case tag_image_length:
            metadata.height = read_single_value(bytes, entry, byte_order);
            break;
        case tag_bits_per_sample:
            metadata.bits_per_sample.clear();
            for (const std::uint32_t value : read_value_vector(bytes, entry, byte_order)) {
                metadata.bits_per_sample.push_back(static_cast<std::uint16_t>(value));
            }
            break;
        case tag_compression:
            metadata.compression = static_cast<std::uint16_t>(read_single_value(bytes, entry, byte_order));
            break;
        case tag_photometric_interpretation:
            metadata.photometric_interpretation =
                static_cast<std::uint16_t>(read_single_value(bytes, entry, byte_order));
            break;
        case tag_strip_offsets:
            metadata.strip_offsets = read_value_vector(bytes, entry, byte_order);
            break;
        case tag_samples_per_pixel:
            metadata.samples_per_pixel = static_cast<std::uint16_t>(read_single_value(bytes, entry, byte_order));
            break;
        case tag_rows_per_strip:
            metadata.rows_per_strip = read_single_value(bytes, entry, byte_order);
            break;
        case tag_strip_byte_counts:
            metadata.strip_byte_counts = read_value_vector(bytes, entry, byte_order);
            break;
        case tag_planar_configuration:
            metadata.planar_configuration =
                static_cast<std::uint16_t>(read_single_value(bytes, entry, byte_order));
            break;
        case tag_sample_format:
            metadata.sample_format.clear();
            for (const std::uint32_t value : read_value_vector(bytes, entry, byte_order)) {
                metadata.sample_format.push_back(static_cast<std::uint16_t>(value));
            }
            break;
        default:
            break;
    }
}

TiffMetadata read_metadata(const std::vector<std::uint8_t>& bytes, const ByteOrder byte_order) {
    const std::uint32_t ifd_offset = read_u32(bytes, 4, byte_order);
    require_range(bytes, ifd_offset, 2);

    const std::uint16_t entry_count = read_u16(bytes, ifd_offset, byte_order);
    const std::size_t entries_offset = static_cast<std::size_t>(ifd_offset) + 2;
    if (entry_count > (bytes.size() - entries_offset) / 12) {
        throw std::runtime_error("TIFF IFD entry table is truncated");
    }

    TiffMetadata metadata;
    for (std::size_t index = 0; index < entry_count; ++index) {
        const std::size_t entry_offset = entries_offset + index * 12;
        TiffEntry entry;
        entry.tag = read_u16(bytes, entry_offset, byte_order);
        entry.type = read_u16(bytes, entry_offset + 2, byte_order);
        entry.count = read_u32(bytes, entry_offset + 4, byte_order);
        entry.value_or_offset = read_u32(bytes, entry_offset + 8, byte_order);
        entry.inline_value_offset = entry_offset + 8;

        apply_entry(bytes, entry, byte_order, metadata);
    }

    return metadata;
}

ByteOrder read_byte_order(const std::vector<std::uint8_t>& bytes) {
    require_range(bytes, 0, 8);

    if (bytes[0] == 'I' && bytes[1] == 'I') {
        return ByteOrder::intel;
    }

    if (bytes[0] == 'M' && bytes[1] == 'M') {
        return ByteOrder::motorola;
    }

    throw std::runtime_error("unsupported TIFF byte order");
}

bool is_rgb_tiff(const TiffMetadata& metadata) {
    return metadata.photometric_interpretation == 2 && metadata.samples_per_pixel == 3;
}

bool is_grayscale_tiff(const TiffMetadata& metadata) {
    return
        metadata.samples_per_pixel == 1 &&
        (metadata.photometric_interpretation == 0 || metadata.photometric_interpretation == 1);
}

void validate_metadata(TiffMetadata& metadata) {
    if (metadata.width == 0 || metadata.height == 0) {
        throw std::runtime_error("TIFF image dimensions are missing or invalid");
    }

    if (metadata.width > std::numeric_limits<std::size_t>::max() / metadata.height) {
        throw std::runtime_error("TIFF image dimensions are too large");
    }

    if (metadata.bits_per_sample.empty()) {
        metadata.bits_per_sample.assign(metadata.samples_per_pixel, 8);
    }

    if (metadata.bits_per_sample.size() == 1 && metadata.samples_per_pixel > 1) {
        metadata.bits_per_sample.assign(metadata.samples_per_pixel, metadata.bits_per_sample.front());
    }

    if (metadata.bits_per_sample.size() != metadata.samples_per_pixel) {
        throw std::runtime_error("TIFF BitsPerSample count does not match SamplesPerPixel");
    }

    for (const std::uint16_t bits_per_sample : metadata.bits_per_sample) {
        if (bits_per_sample != 8) {
            throw std::runtime_error("only 8-bit TIFF samples are supported");
        }
    }

    if (metadata.compression != 1) {
        throw std::runtime_error("only uncompressed TIFF files are supported");
    }

    if (!is_grayscale_tiff(metadata) && !is_rgb_tiff(metadata)) {
        throw std::runtime_error("only grayscale and RGB TIFF files are supported");
    }

    if (metadata.planar_configuration != 1) {
        throw std::runtime_error("unsupported TIFF planar configuration");
    }

    if (metadata.sample_format.empty()) {
        metadata.sample_format.assign(metadata.samples_per_pixel, 1);
    }

    if (metadata.sample_format.size() == 1 && metadata.samples_per_pixel > 1) {
        metadata.sample_format.assign(metadata.samples_per_pixel, metadata.sample_format.front());
    }

    if (metadata.sample_format.size() != metadata.samples_per_pixel) {
        throw std::runtime_error("TIFF SampleFormat count does not match SamplesPerPixel");
    }

    for (const std::uint16_t sample_format : metadata.sample_format) {
        if (sample_format != 1) {
            throw std::runtime_error("only unsigned integer TIFF samples are supported");
        }
    }

    if (metadata.rows_per_strip == 0) {
        metadata.rows_per_strip = metadata.height;
    }

    if (metadata.strip_offsets.empty() || metadata.strip_byte_counts.empty()) {
        throw std::runtime_error("TIFF strip offsets or byte counts are missing");
    }

    if (metadata.strip_offsets.size() != metadata.strip_byte_counts.size()) {
        throw std::runtime_error("TIFF strip offsets and byte counts have different sizes");
    }
}

struct DecodedTiff {
    TiffMetadata metadata;
    std::vector<std::uint8_t> pixels;
};

std::size_t samples_per_output_pixel(const TiffMetadata& metadata) {
    return is_rgb_tiff(metadata) ? 3 : 1;
}

DecodedTiff read_uncompressed_tiff_pixels(const std::string& path) {
    const std::vector<std::uint8_t> bytes = read_file_bytes(path);
    const ByteOrder byte_order = read_byte_order(bytes);

    if (read_u16(bytes, 2, byte_order) != 42) {
        throw std::runtime_error("invalid TIFF magic number");
    }

    TiffMetadata metadata = read_metadata(bytes, byte_order);
    validate_metadata(metadata);

    const std::size_t width = static_cast<std::size_t>(metadata.width);
    const std::size_t height = static_cast<std::size_t>(metadata.height);
    const std::size_t sample_count = samples_per_output_pixel(metadata);
    std::size_t output_offset = 0;
    const std::size_t expected_bytes = width * height * sample_count;
    std::vector<std::uint8_t> pixels(expected_bytes);

    for (std::size_t strip_index = 0; strip_index < metadata.strip_offsets.size(); ++strip_index) {
        const std::size_t strip_offset = metadata.strip_offsets[strip_index];
        const std::size_t byte_count = metadata.strip_byte_counts[strip_index];
        require_range(bytes, strip_offset, byte_count);

        const std::size_t bytes_to_copy = std::min(byte_count, expected_bytes - output_offset);
        std::copy(
            bytes.begin() + static_cast<std::ptrdiff_t>(strip_offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(strip_offset + bytes_to_copy),
            pixels.begin() + static_cast<std::ptrdiff_t>(output_offset)
        );

        output_offset += bytes_to_copy;
        if (output_offset == expected_bytes) {
            break;
        }
    }

    if (output_offset != expected_bytes) {
        throw std::runtime_error("TIFF strips do not contain enough image data");
    }

    return {metadata, std::move(pixels)};
}

GrayImage::Pixel rgb_to_gray(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue) {
    const int value = 299 * static_cast<int>(red) + 587 * static_cast<int>(green) + 114 * static_cast<int>(blue);
    return static_cast<GrayImage::Pixel>((value + 500) / 1000);
}

} // namespace

GrayImage read_uncompressed_tiff_gray_image(const std::string& path) {
    const DecodedTiff decoded = read_uncompressed_tiff_pixels(path);
    const std::size_t width = static_cast<std::size_t>(decoded.metadata.width);
    const std::size_t height = static_cast<std::size_t>(decoded.metadata.height);
    GrayImage image(width, height);

    if (is_rgb_tiff(decoded.metadata)) {
        for (std::size_t index = 0; index < image.size(); ++index) {
            const std::size_t offset = index * 3;
            image.pixels()[index] = rgb_to_gray(
                decoded.pixels[offset],
                decoded.pixels[offset + 1],
                decoded.pixels[offset + 2]
            );
        }

        return image;
    }

    for (std::size_t index = 0; index < image.size(); ++index) {
        const std::uint8_t value = decoded.pixels[index];
        image.pixels()[index] =
            decoded.metadata.photometric_interpretation == 0
                ? static_cast<GrayImage::Pixel>(255 - value)
                : static_cast<GrayImage::Pixel>(value);
    }

    return image;
}

RgbImage read_uncompressed_tiff_rgb_image(const std::string& path) {
    const DecodedTiff decoded = read_uncompressed_tiff_pixels(path);
    RgbImage image;
    image.width = static_cast<std::size_t>(decoded.metadata.width);
    image.height = static_cast<std::size_t>(decoded.metadata.height);
    image.pixels.resize(image.width * image.height);

    if (is_rgb_tiff(decoded.metadata)) {
        for (std::size_t index = 0; index < image.pixels.size(); ++index) {
            const std::size_t offset = index * 3;
            image.pixels[index] = {
                decoded.pixels[offset],
                decoded.pixels[offset + 1],
                decoded.pixels[offset + 2],
            };
        }

        return image;
    }

    for (std::size_t index = 0; index < image.pixels.size(); ++index) {
        const std::uint8_t value = decoded.metadata.photometric_interpretation == 0
            ? static_cast<std::uint8_t>(255 - decoded.pixels[index])
            : decoded.pixels[index];
        image.pixels[index] = {value, value, value};
    }

    return image;
}

} // namespace dip::homework
