#include "app/cli.hpp"

#include "core/image_io.hpp"
#include "lab01_analysis/cooccurrence.hpp"
#include "lab01_analysis/noise.hpp"
#include "lab01_analysis/quality.hpp"
#include "lab01_analysis/statistics.hpp"
#include "lab02_fourier/fft.hpp"
#include "lab03_geometry/rotation.hpp"
#include "lab04_convolution/convolution.hpp"
#include "lab04_convolution/high_pass.hpp"
#include "lab04_convolution/low_pass.hpp"
#include "lab05_nonlinear/morphology.hpp"
#include "lab05_nonlinear/rank_filter.hpp"
#include "lab06_segmentation/connected_components.hpp"
#include "lab06_segmentation/otsu.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace dip {
namespace {

void print_general_help(std::ostream& output) {
    output << "Usage:\n"
           << "  dip info --input <path>\n"
           << "  dip lab1 stats --input <path> --output <path> [--histogram-output <path>]\n"
           << "  dip lab1 glcm --input <path> --output <path> --dr <rows> --dc <columns> [--matrix-output <path>]\n"
           << "  dip lab1 noise --input <path> --output <path> --variance <value> [--seed <value>]\n"
           << "  dip lab1 psnr --original <path> --distorted <path> --output <path>\n"
           << "  dip lab2 signal-plot --input <path> --output <path>\n"
           << "  dip lab2 signal-spectrum --input <path> --output <path>\n"
           << "  dip lab2 image-spectrum --input <path> --output <path>\n"
           << "  dip lab3 rotate --input <path> --output <path> --angle <degrees> [--method <name>]\n"
           << "  dip lab3 compare --input <path> --output <path> --angle <degrees>\n"
           << "  dip lab4 convolve --input <path> --output <path> --kernel <path>\n"
           << "  dip lab4 threshold-lowpass --input <path> --output <path> --kernel-size <odd> --threshold <value>\n"
           << "  dip lab4 lowpass-denoise --input <path> --noise <gaussian|impulse> --kernel-size <odd> --noisy-output <path> --filtered-output <path> --metrics-output <path> [--threshold <value>]\n"
           << "  dip lab4 laplacian --input <path> --output <path> [--kernel <four|eight>] [--kernel-size <odd>]\n"
           << "  dip lab4 log-filter --input <path> --output <path> --kernel-size <odd> --sigma <value>\n"
           << "  dip lab4 zero-crossing --input <path> --output <path> --kernel-size <odd> --sigma <value> [--metrics-output <path>]\n"
           << "  dip lab4 sharpen --input <path> --output <path> [--kernel-size <odd>] [--amount <value>]\n"
           << "  dip lab5 rank --input <path> --output <path> --aperture <path> --rank <index|min|median|max>\n"
           << "  dip lab5 trimmed-mean --input <path> --output <path> --aperture <path> --trimmed-count <even>\n"
           << "  dip lab5 morphology --input <path> --output <path> --aperture <path> --operation <erosion|dilation|opening|closing>\n"
           << "  dip lab5 compare-denoise --input <path> --noise <gaussian|impulse> --aperture <path> --trimmed-count <even> --noisy-output <path> --median-output <path> --trimmed-output <path> --metrics-output <path>\n"
           << "  dip lab6 otsu --input <path> --output <path> [--metrics-output <path>]\n"
           << "  dip lab6 components --input <path> --output <path> [--metrics-output <path>]\n"
           << "  dip lab6 circles --input <path> --output <path> [--metrics-output <path>]\n"
           << "  dip lab1 --help\n"
           << "  dip lab2 --help\n"
           << "  dip lab3 --help\n"
           << "  dip lab4 --help\n"
           << "  dip lab5 --help\n"
           << "  dip lab6 --help\n";
}

void print_lab_help(std::ostream& output, const std::string& lab) {
    output << "Usage:\n";

    if (lab == "lab1") {
        output << "  dip lab1 stats --input <path> --output <path> [--histogram-output <path>]\n"
               << "  dip lab1 glcm --input <path> --output <path> --dr <rows> --dc <columns> [--matrix-output <path>]\n"
               << "  dip lab1 noise --input <path> --output <path> --variance <value> [--seed <value>]\n"
               << "  dip lab1 psnr --original <path> --distorted <path> --output <path>\n"
               << "  dip lab1 --help\n\n"
               << "Available commands:\n"
               << "  stats  Calculate grayscale image statistics and optionally save a histogram image.\n"
               << "  glcm   Calculate a gray-level co-occurrence matrix and optionally save its image.\n"
               << "  noise  Add white Gaussian noise with a specified variance.\n"
               << "  psnr   Calculate MSE and PSNR for two grayscale images.\n";
        return;
    }

    if (lab == "lab2") {
        output << "  dip lab2 signal-plot --input <path> --output <path>\n"
               << "  dip lab2 signal-spectrum --input <path> --output <path>\n"
               << "  dip lab2 image-spectrum --input <path> --output <path>\n"
               << "  dip lab2 --help\n\n"
               << "Available commands:\n"
               << "  signal-plot      Draw a signal waveform image.\n"
               << "  signal-spectrum  Calculate and save a centered log amplitude spectrum for a signal.\n"
               << "  image-spectrum   Calculate and save a centered log amplitude spectrum for an image.\n";
        return;
    }

    if (lab == "lab3") {
        output << "  dip lab3 rotate --input <path> --output <path> --angle <degrees> [--method <name>]\n"
               << "  dip lab3 compare --input <path> --output <path> --angle <degrees>\n"
               << "  dip lab3 --help\n\n"
               << "Available commands:\n"
               << "  rotate   Rotate a grayscale image using nearest, bilinear, or bicubic interpolation.\n"
               << "  compare  Compare rotation time and round-trip PSNR/MSE for all interpolation methods.\n";
        return;
    }

    if (lab == "lab4") {
        output << "  dip lab4 convolve --input <path> --output <path> --kernel <path>\n"
               << "  dip lab4 threshold-lowpass --input <path> --output <path> --kernel-size <odd> --threshold <value>\n"
               << "  dip lab4 lowpass-denoise --input <path> --noise <gaussian|impulse> --kernel-size <odd> --noisy-output <path> --filtered-output <path> --metrics-output <path> [--threshold <value>]\n"
               << "  dip lab4 laplacian --input <path> --output <path> [--kernel <four|eight>] [--kernel-size <odd>]\n"
               << "  dip lab4 log-filter --input <path> --output <path> --kernel-size <odd> --sigma <value>\n"
               << "  dip lab4 zero-crossing --input <path> --output <path> --kernel-size <odd> --sigma <value> [--metrics-output <path>]\n"
               << "  dip lab4 sharpen --input <path> --output <path> [--kernel-size <odd>] [--amount <value>]\n"
               << "  dip lab4 --help\n\n"
               << "Available commands:\n"
               << "  convolve          Apply a 2D spatial convolution kernel and drop boundary cells.\n"
               << "  threshold-lowpass Apply an averaging low-pass filter only when the change reaches a threshold.\n"
               << "  lowpass-denoise   Add noise, apply an averaging low-pass filter, and save PSNR metrics.\n"
               << "  laplacian         Save a normalized Laplacian response magnitude image for a chosen odd size.\n"
               << "  log-filter        Save a normalized Laplacian-of-Gaussian response magnitude image.\n"
               << "  zero-crossing     Save a binary edge map from LoG zero crossings.\n"
               << "  sharpen           Apply a sharpening filter of a chosen odd size.\n";
        return;
    }

    if (lab == "lab5") {
        output << "  dip lab5 rank --input <path> --output <path> --aperture <path> --rank <index|min|median|max>\n"
               << "  dip lab5 trimmed-mean --input <path> --output <path> --aperture <path> --trimmed-count <even>\n"
               << "  dip lab5 morphology --input <path> --output <path> --aperture <path> --operation <erosion|dilation|opening|closing>\n"
               << "  dip lab5 compare-denoise --input <path> --noise <gaussian|impulse> --aperture <path> --trimmed-count <even> --noisy-output <path> --median-output <path> --trimmed-output <path> --metrics-output <path>\n"
               << "  dip lab5 --help\n\n"
               << "Available commands:\n"
               << "  rank             Apply rank filtering with an arbitrary aperture mask.\n"
               << "  trimmed-mean     Apply trimmed mean filtering with an arbitrary aperture mask.\n"
               << "  morphology       Apply grayscale erosion, dilation, opening, or closing.\n"
               << "  compare-denoise  Compare median and trimmed mean filters by PSNR.\n";
        return;
    }

    if (lab == "lab6") {
        output << "  dip lab6 otsu --input <path> --output <path> [--metrics-output <path>]\n"
               << "  dip lab6 components --input <path> --output <path> [--metrics-output <path>]\n"
               << "  dip lab6 circles --input <path> --output <path> [--metrics-output <path>]\n"
               << "  dip lab6 --help\n\n"
               << "Available commands:\n"
               << "  otsu        Binarize a grayscale image using an automatically selected Otsu threshold.\n"
               << "  components  Label 4-connected nonzero regions in a binary image.\n"
               << "  circles     Highlight large 4-connected regions with circle-like shape.\n";
        return;
    }

    output << "  dip " << lab << " --help\n\n"
           << "The " << lab << " command is reserved for a future laboratory task.\n";
}

bool is_lab_command(const std::string& command) {
    return command == "lab1" ||
           command == "lab2" ||
           command == "lab3" ||
           command == "lab4" ||
           command == "lab5" ||
           command == "lab6";
}

int run_info(const std::vector<std::string>& args) {
    std::string input_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for info: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    std::cout << "path: " << input_path << '\n'
              << "width: " << image.width() << '\n'
              << "height: " << image.height() << '\n'
              << "channels: 1\n"
              << "pixel_type: uint8\n";

    return 0;
}

bool parse_int(const std::string& text, int& value) {
    try {
        std::size_t parsed = 0;
        value = std::stoi(text, &parsed);
        return parsed == text.size();
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_double(const std::string& text, double& value) {
    try {
        std::size_t parsed = 0;
        value = std::stod(text, &parsed);
        return parsed == text.size();
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_seed(const std::string& text, std::uint32_t& value) {
    try {
        std::size_t parsed = 0;
        const unsigned long parsed_value = std::stoul(text, &parsed);
        if (parsed != text.size() || parsed_value > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }

        value = static_cast<std::uint32_t>(parsed_value);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_interpolation_method(const std::string& text, lab03::InterpolationMethod& method) {
    if (text == "nearest") {
        method = lab03::InterpolationMethod::nearest;
        return true;
    }

    if (text == "bilinear") {
        method = lab03::InterpolationMethod::bilinear;
        return true;
    }

    if (text == "bicubic") {
        method = lab03::InterpolationMethod::bicubic;
        return true;
    }

    return false;
}

enum class NoiseKind {
    gaussian,
    impulse,
};

bool parse_noise_kind(const std::string& text, NoiseKind& kind) {
    if (text == "gaussian") {
        kind = NoiseKind::gaussian;
        return true;
    }

    if (text == "impulse") {
        kind = NoiseKind::impulse;
        return true;
    }

    return false;
}

const char* noise_kind_name(const NoiseKind kind) noexcept {
    switch (kind) {
        case NoiseKind::gaussian:
            return "gaussian";
        case NoiseKind::impulse:
            return "impulse";
    }

    return "unknown";
}

bool parse_laplacian_kernel(const std::string& text, lab04::LaplacianKernel& kernel) {
    if (text == "four") {
        kernel = lab04::LaplacianKernel::four_neighbor;
        return true;
    }

    if (text == "eight") {
        kernel = lab04::LaplacianKernel::eight_neighbor;
        return true;
    }

    return false;
}

void write_statistics_json(
    const std::string& output_path,
    const std::string& input_path,
    const GrayImage& image,
    const lab01::ImageStatistics& stats
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(10);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"width\": " << image.width() << ",\n"
           << "  \"height\": " << image.height() << ",\n"
           << "  \"channels\": 1,\n"
           << "  \"pixel_type\": \"uint8\",\n"
           << "  \"mean\": " << stats.mean << ",\n"
           << "  \"variance\": " << stats.variance << ",\n"
           << "  \"quartiles\": {\n"
           << "    \"q1\": " << stats.quartiles.q1 << ",\n"
           << "    \"q2\": " << stats.quartiles.q2 << ",\n"
           << "    \"q3\": " << stats.quartiles.q3 << "\n"
           << "  },\n"
           << "  \"entropy\": " << stats.entropy << ",\n"
           << "  \"energy\": " << stats.energy << ",\n"
           << "  \"skewness\": " << stats.skewness << ",\n"
           << "  \"kurtosis\": " << stats.kurtosis << ",\n"
           << "  \"histogram\": [";

    for (std::size_t i = 0; i < stats.histogram.size(); ++i) {
        if (i != 0) {
            output << ", ";
        }
        output << stats.histogram[i];
    }

    output << "]\n"
           << "}\n";
}

int run_lab1_stats(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string histogram_output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--histogram-output" && i + 1 < args.size()) {
            histogram_output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab1 stats: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const lab01::ImageStatistics stats = lab01::calculate_statistics(image);
    write_statistics_json(output_path, input_path, image, stats);

    if (!histogram_output_path.empty()) {
        write_gray_image(histogram_output_path, lab01::render_histogram_image(stats.histogram));
    }

    return 0;
}

void write_cooccurrence_json(
    const std::string& output_path,
    const std::string& input_path,
    const GrayImage& image,
    const lab01::CooccurrenceMatrix& matrix
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"width\": " << image.width() << ",\n"
           << "  \"height\": " << image.height() << ",\n"
           << "  \"row_offset\": " << matrix.row_offset << ",\n"
           << "  \"column_offset\": " << matrix.column_offset << ",\n"
           << "  \"total_pairs\": " << matrix.total_pairs << ",\n"
           << "  \"energy\": " << lab01::cooccurrence_energy(matrix) << ",\n"
           << "  \"matrix\": [\n";

    for (std::size_t first = 0; first < lab01::cooccurrence_levels; ++first) {
        output << "    [";

        for (std::size_t second = 0; second < lab01::cooccurrence_levels; ++second) {
            if (second != 0) {
                output << ", ";
            }

            output << matrix.counts[first * lab01::cooccurrence_levels + second];
        }

        output << "]";
        if (first + 1 != lab01::cooccurrence_levels) {
            output << ",";
        }
        output << "\n";
    }

    output << "  ]\n"
           << "}\n";
}

void write_psnr_json(
    const std::string& output_path,
    const std::string& original_path,
    const std::string& distorted_path,
    const GrayImage& original,
    const GrayImage& distorted
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    const double mse = lab01::mean_squared_error(original, distorted);
    const double psnr = lab01::peak_signal_to_noise_ratio(original, distorted);

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"original\": \"" << original_path << "\",\n"
           << "  \"distorted\": \"" << distorted_path << "\",\n"
           << "  \"width\": " << original.width() << ",\n"
           << "  \"height\": " << original.height() << ",\n"
           << "  \"mse\": " << mse << ",\n"
           << "  \"psnr_db\": ";

    if (std::isinf(psnr)) {
        output << "\"inf\"";
    } else {
        output << psnr;
    }

    output << "\n"
           << "}\n";
}

void write_psnr_value(std::ostream& output, const double value) {
    if (std::isinf(value)) {
        output << "\"inf\"";
    } else {
        output << value;
    }
}

std::vector<double> read_signal_samples(const std::string& input_path) {
    std::ifstream input(input_path);
    if (!input) {
        throw std::runtime_error("failed to open signal file: " + input_path);
    }

    std::vector<double> samples;
    std::string line;

    while (std::getline(input, line)) {
        for (char& symbol : line) {
            if (symbol == ',' || symbol == ';') {
                symbol = ' ';
            }
        }

        std::istringstream line_stream(line);
        double value = 0.0;

        while (line_stream >> value) {
            samples.push_back(value);
        }
    }

    if (samples.empty()) {
        throw std::runtime_error("signal file contains no samples: " + input_path);
    }

    return samples;
}

lab04::Kernel2D read_kernel_file(const std::string& input_path) {
    std::ifstream input(input_path);
    if (!input) {
        throw std::runtime_error("failed to open kernel file: " + input_path);
    }

    std::vector<double> values;
    std::size_t width = 0;
    std::size_t height = 0;
    std::string line;

    while (std::getline(input, line)) {
        for (char& symbol : line) {
            if (symbol == ',' || symbol == ';') {
                symbol = ' ';
            }
        }

        std::istringstream line_stream(line);
        std::vector<double> row;
        double value = 0.0;

        while (line_stream >> value) {
            if (!std::isfinite(value)) {
                throw std::runtime_error("kernel file contains a non-finite number: " + input_path);
            }

            row.push_back(value);
        }

        if (!line_stream.eof()) {
            throw std::runtime_error("kernel file contains an invalid number: " + input_path);
        }

        if (row.empty()) {
            continue;
        }

        if (width == 0) {
            width = row.size();
        } else if (row.size() != width) {
            throw std::runtime_error("kernel rows must have the same length: " + input_path);
        }

        values.insert(values.end(), row.begin(), row.end());
        ++height;
    }

    if (width == 0 || height == 0) {
        throw std::runtime_error("kernel file contains no values: " + input_path);
    }

    return {width, height, values};
}

lab05::Aperture read_aperture_file(const std::string& input_path) {
    std::ifstream input(input_path);
    if (!input) {
        throw std::runtime_error("failed to open aperture file: " + input_path);
    }

    std::vector<std::uint8_t> mask;
    std::size_t width = 0;
    std::size_t height = 0;
    std::string line;

    while (std::getline(input, line)) {
        for (char& symbol : line) {
            if (symbol == ',' || symbol == ';') {
                symbol = ' ';
            }
        }

        std::istringstream line_stream(line);
        std::vector<std::uint8_t> row;
        int value = 0;

        while (line_stream >> value) {
            if (value != 0 && value != 1) {
                throw std::runtime_error("aperture file values must be 0 or 1: " + input_path);
            }

            row.push_back(static_cast<std::uint8_t>(value));
        }

        if (!line_stream.eof()) {
            throw std::runtime_error("aperture file contains an invalid value: " + input_path);
        }

        if (row.empty()) {
            continue;
        }

        if (width == 0) {
            width = row.size();
        } else if (row.size() != width) {
            throw std::runtime_error("aperture rows must have the same length: " + input_path);
        }

        mask.insert(mask.end(), row.begin(), row.end());
        ++height;
    }

    if (width == 0 || height == 0) {
        throw std::runtime_error("aperture file contains no values: " + input_path);
    }

    return {width, height, mask};
}

bool parse_rank(const std::string& text, const std::size_t active_size, std::size_t& rank) {
    if (active_size == 0) {
        return false;
    }

    if (text == "min") {
        rank = 0;
        return true;
    }

    if (text == "median") {
        rank = active_size / 2;
        return true;
    }

    if (text == "max") {
        rank = active_size - 1;
        return true;
    }

    try {
        std::size_t parsed = 0;
        const unsigned long long value = std::stoull(text, &parsed);
        if (parsed != text.size() || value >= active_size) {
            return false;
        }

        rank = static_cast<std::size_t>(value);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

enum class MorphologyOperation {
    erosion,
    dilation,
    opening,
    closing
};

bool parse_morphology_operation(const std::string& text, MorphologyOperation& operation) {
    if (text == "erosion" || text == "erode") {
        operation = MorphologyOperation::erosion;
        return true;
    }

    if (text == "dilation" || text == "dilate") {
        operation = MorphologyOperation::dilation;
        return true;
    }

    if (text == "opening" || text == "open") {
        operation = MorphologyOperation::opening;
        return true;
    }

    if (text == "closing" || text == "close") {
        operation = MorphologyOperation::closing;
        return true;
    }

    return false;
}

GrayImage center_crop_or_pad(
    const GrayImage& image,
    const std::size_t width,
    const std::size_t height,
    const GrayImage::Pixel background = 0
) {
    GrayImage result(width, height, std::vector<GrayImage::Pixel>(width * height, background));
    const long offset_x = (static_cast<long>(image.width()) - static_cast<long>(width)) / 2;
    const long offset_y = (static_cast<long>(image.height()) - static_cast<long>(height)) / 2;

    for (std::size_t y = 0; y < height; ++y) {
        const long source_y = static_cast<long>(y) + offset_y;
        if (source_y < 0 || source_y >= static_cast<long>(image.height())) {
            continue;
        }

        for (std::size_t x = 0; x < width; ++x) {
            const long source_x = static_cast<long>(x) + offset_x;
            if (source_x < 0 || source_x >= static_cast<long>(image.width())) {
                continue;
            }

            result.at(x, y) = image.at(static_cast<std::size_t>(source_x), static_cast<std::size_t>(source_y));
        }
    }

    return result;
}

void write_lowpass_denoise_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& noisy_output_path,
    const std::string& filtered_output_path,
    const NoiseKind noise_kind,
    const std::string& noise_parameter_name,
    const double noise_parameter_value,
    const std::uint32_t seed,
    const std::size_t kernel_size,
    const bool has_filter_threshold,
    const double filter_threshold,
    const GrayImage& original,
    const GrayImage& original_crop,
    const GrayImage& noisy_crop,
    const GrayImage& filtered
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    const double noisy_mse = lab01::mean_squared_error(original_crop, noisy_crop);
    const double noisy_psnr = lab01::peak_signal_to_noise_ratio(original_crop, noisy_crop);
    const double filtered_mse = lab01::mean_squared_error(original_crop, filtered);
    const double filtered_psnr = lab01::peak_signal_to_noise_ratio(original_crop, filtered);

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"noisy_output\": \"" << noisy_output_path << "\",\n"
           << "  \"filtered_output\": \"" << filtered_output_path << "\",\n"
           << "  \"noise_type\": \"" << noise_kind_name(noise_kind) << "\",\n"
           << "  \"" << noise_parameter_name << "\": " << noise_parameter_value << ",\n"
           << "  \"seed\": " << seed << ",\n"
           << "  \"filter\": {\n"
           << "    \"type\": \"" << (has_filter_threshold ? "thresholded_average_low_pass" : "average_low_pass") << "\",\n"
           << "    \"kernel_width\": " << kernel_size << ",\n"
           << "    \"kernel_height\": " << kernel_size;
    if (has_filter_threshold) {
        output << ",\n"
               << "    \"threshold\": " << filter_threshold << "\n";
    } else {
        output << "\n";
    }
    output << "  },\n"
           << "  \"original_width\": " << original.width() << ",\n"
           << "  \"original_height\": " << original.height() << ",\n"
           << "  \"comparison_width\": " << filtered.width() << ",\n"
           << "  \"comparison_height\": " << filtered.height() << ",\n"
           << "  \"comparison_method\": \"center crop original and noisy images to the valid convolution result size\",\n"
           << "  \"noisy_mse\": " << noisy_mse << ",\n"
           << "  \"noisy_psnr_db\": ";
    write_psnr_value(output, noisy_psnr);
    output << ",\n"
           << "  \"filtered_mse\": " << filtered_mse << ",\n"
           << "  \"filtered_psnr_db\": ";
    write_psnr_value(output, filtered_psnr);
    output << "\n"
           << "}\n";
}

void write_lab5_denoise_comparison_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& noisy_output_path,
    const std::string& median_output_path,
    const std::string& trimmed_output_path,
    const NoiseKind noise_kind,
    const std::string& noise_parameter_name,
    const double noise_parameter_value,
    const std::uint32_t seed,
    const lab05::Aperture& aperture,
    const std::size_t median_rank,
    const std::size_t trimmed_count,
    const GrayImage& original,
    const GrayImage& original_crop,
    const GrayImage& noisy_crop,
    const GrayImage& median_filtered,
    const GrayImage& trimmed_filtered
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    const double noisy_mse = lab01::mean_squared_error(original_crop, noisy_crop);
    const double noisy_psnr = lab01::peak_signal_to_noise_ratio(original_crop, noisy_crop);
    const double median_mse = lab01::mean_squared_error(original_crop, median_filtered);
    const double median_psnr = lab01::peak_signal_to_noise_ratio(original_crop, median_filtered);
    const double trimmed_mse = lab01::mean_squared_error(original_crop, trimmed_filtered);
    const double trimmed_psnr = lab01::peak_signal_to_noise_ratio(original_crop, trimmed_filtered);

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"noisy_output\": \"" << noisy_output_path << "\",\n"
           << "  \"median_output\": \"" << median_output_path << "\",\n"
           << "  \"trimmed_mean_output\": \"" << trimmed_output_path << "\",\n"
           << "  \"noise_type\": \"" << noise_kind_name(noise_kind) << "\",\n"
           << "  \"" << noise_parameter_name << "\": " << noise_parameter_value << ",\n"
           << "  \"seed\": " << seed << ",\n"
           << "  \"aperture\": {\n"
           << "    \"width\": " << aperture.width << ",\n"
           << "    \"height\": " << aperture.height << ",\n"
           << "    \"active_cells\": " << lab05::active_aperture_size(aperture) << "\n"
           << "  },\n"
           << "  \"median_filter\": {\n"
           << "    \"rank\": " << median_rank << "\n"
           << "  },\n"
           << "  \"trimmed_mean_filter\": {\n"
           << "    \"trimmed_count\": " << trimmed_count << ",\n"
           << "    \"trim_each_side\": " << (trimmed_count / 2) << "\n"
           << "  },\n"
           << "  \"original_width\": " << original.width() << ",\n"
           << "  \"original_height\": " << original.height() << ",\n"
           << "  \"comparison_width\": " << median_filtered.width() << ",\n"
           << "  \"comparison_height\": " << median_filtered.height() << ",\n"
           << "  \"comparison_method\": \"center crop original and noisy images to the valid aperture result size\",\n"
           << "  \"noisy_mse\": " << noisy_mse << ",\n"
           << "  \"noisy_psnr_db\": ";
    write_psnr_value(output, noisy_psnr);
    output << ",\n"
           << "  \"median_mse\": " << median_mse << ",\n"
           << "  \"median_psnr_db\": ";
    write_psnr_value(output, median_psnr);
    output << ",\n"
           << "  \"trimmed_mean_mse\": " << trimmed_mse << ",\n"
           << "  \"trimmed_mean_psnr_db\": ";
    write_psnr_value(output, trimmed_psnr);
    output << "\n"
           << "}\n";
}

std::size_t count_edge_pixels(const GrayImage& image) {
    std::size_t count = 0;
    for (const GrayImage::Pixel pixel : image.pixels()) {
        if (pixel != 0) {
            ++count;
        }
    }

    return count;
}

void write_zero_crossing_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& edge_output_path,
    const GrayImage& original,
    const GrayImage& edges,
    const std::size_t kernel_size,
    const double sigma,
    const double threshold
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"edge_output\": \"" << edge_output_path << "\",\n"
           << "  \"method\": \"LoG zero crossing\",\n"
           << "  \"kernel_width\": " << kernel_size << ",\n"
           << "  \"kernel_height\": " << kernel_size << ",\n"
           << "  \"sigma\": " << sigma << ",\n"
           << "  \"threshold\": " << threshold << ",\n"
           << "  \"threshold_method\": \"3 * sum(abs(I_e)) / (4 * W * H)\",\n"
           << "  \"original_width\": " << original.width() << ",\n"
           << "  \"original_height\": " << original.height() << ",\n"
           << "  \"edge_width\": " << edges.width() << ",\n"
           << "  \"edge_height\": " << edges.height() << ",\n"
           << "  \"edge_pixels\": " << count_edge_pixels(edges) << "\n"
           << "}\n";
}

void write_otsu_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& binary_output_path,
    const GrayImage& image,
    const lab06::OtsuResult& result
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"binary_output\": \"" << binary_output_path << "\",\n"
           << "  \"method\": \"Otsu thresholding\",\n"
           << "  \"width\": " << image.width() << ",\n"
           << "  \"height\": " << image.height() << ",\n"
           << "  \"threshold\": " << static_cast<int>(result.threshold) << ",\n"
           << "  \"class_0\": {\n"
           << "    \"condition\": \"pixel <= threshold\",\n"
           << "    \"output_value\": 0,\n"
           << "    \"pixel_count\": " << result.class_pixel_counts[0] << ",\n"
           << "    \"probability\": " << result.class_probabilities[0] << ",\n"
           << "    \"mean\": " << result.class_means[0] << "\n"
           << "  },\n"
           << "  \"class_1\": {\n"
           << "    \"condition\": \"pixel > threshold\",\n"
           << "    \"output_value\": 255,\n"
           << "    \"pixel_count\": " << result.class_pixel_counts[1] << ",\n"
           << "    \"probability\": " << result.class_probabilities[1] << ",\n"
           << "    \"mean\": " << result.class_means[1] << "\n"
           << "  },\n"
           << "  \"within_class_variance\": " << result.within_class_variance << ",\n"
           << "  \"between_class_variance\": " << result.between_class_variance << "\n"
           << "}\n";
}

void write_connected_components_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& labels_output_path,
    const lab06::ConnectedComponentsResult& result
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"labels_output\": \"" << labels_output_path << "\",\n"
           << "  \"method\": \"4-connected component labeling\",\n"
           << "  \"foreground_condition\": \"pixel != 0\",\n"
           << "  \"width\": " << result.width << ",\n"
           << "  \"height\": " << result.height << ",\n"
           << "  \"component_count\": " << result.components.size() << ",\n"
           << "  \"components\": [\n";

    for (std::size_t i = 0; i < result.components.size(); ++i) {
        const lab06::ConnectedComponent& component = result.components[i];
        output << "    {\n"
               << "      \"label\": " << component.label << ",\n"
               << "      \"area\": " << component.area << ",\n"
               << "      \"min_x\": " << component.min_x << ",\n"
               << "      \"min_y\": " << component.min_y << ",\n"
               << "      \"max_x\": " << component.max_x << ",\n"
               << "      \"max_y\": " << component.max_y << ",\n"
               << "      \"raw_moments\": {\n"
               << "        \"m00\": " << component.m00 << ",\n"
               << "        \"m10\": " << component.m10 << ",\n"
               << "        \"m01\": " << component.m01 << "\n"
               << "      },\n"
               << "      \"centroid\": {\n"
               << "        \"x\": " << component.centroid_x << ",\n"
               << "        \"y\": " << component.centroid_y << "\n"
               << "      },\n"
               << "      \"central_moments\": {\n"
               << "        \"mu20\": " << component.mu20 << ",\n"
               << "        \"mu02\": " << component.mu02 << ",\n"
               << "        \"mu11\": " << component.mu11 << "\n"
               << "      }\n"
               << "    }";
        if (i + 1 < result.components.size()) {
            output << ',';
        }
        output << '\n';
    }

    output << "  ]\n"
           << "}\n";
}

void write_circle_like_components_json(
    const std::string& output_path,
    const std::string& input_path,
    const std::string& highlighted_output_path,
    const lab06::ConnectedComponentsResult& result,
    const std::vector<lab06::CircleLikeComponent>& selected_components,
    const std::size_t min_area,
    const double max_roundness_deviation
) {
    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"highlighted_output\": \"" << highlighted_output_path << "\",\n"
           << "  \"method\": \"large circle-like component detection\",\n"
           << "  \"foreground_condition\": \"pixel != 0\",\n"
           << "  \"width\": " << result.width << ",\n"
           << "  \"height\": " << result.height << ",\n"
           << "  \"component_count\": " << result.components.size() << ",\n"
           << "  \"selected_count\": " << selected_components.size() << ",\n"
           << "  \"criteria\": {\n"
           << "    \"min_area_exclusive\": " << min_area << ",\n"
           << "    \"roundness_kr2_formula\": \"2 * pi * (mu20 + mu02) / S^2\",\n"
           << "    \"target_roundness_kr2\": 1.000000000000,\n"
           << "    \"max_roundness_deviation\": " << max_roundness_deviation << "\n"
           << "  },\n"
           << "  \"selected_components\": [\n";

    for (std::size_t i = 0; i < selected_components.size(); ++i) {
        const lab06::CircleLikeComponent& selected = selected_components[i];
        const lab06::ConnectedComponent& component = selected.component;
        output << "    {\n"
               << "      \"label\": " << component.label << ",\n"
               << "      \"area\": " << component.area << ",\n"
               << "      \"min_x\": " << component.min_x << ",\n"
               << "      \"min_y\": " << component.min_y << ",\n"
               << "      \"max_x\": " << component.max_x << ",\n"
               << "      \"max_y\": " << component.max_y << ",\n"
               << "      \"roundness_kr2\": " << selected.roundness_kr2 << ",\n"
               << "      \"roundness_deviation\": " << selected.roundness_deviation << ",\n"
               << "      \"raw_moments\": {\n"
               << "        \"m00\": " << component.m00 << ",\n"
               << "        \"m10\": " << component.m10 << ",\n"
               << "        \"m01\": " << component.m01 << "\n"
               << "      },\n"
               << "      \"centroid\": {\n"
               << "        \"x\": " << component.centroid_x << ",\n"
               << "        \"y\": " << component.centroid_y << "\n"
               << "      },\n"
               << "      \"central_moments\": {\n"
               << "        \"mu20\": " << component.mu20 << ",\n"
               << "        \"mu02\": " << component.mu02 << ",\n"
               << "        \"mu11\": " << component.mu11 << "\n"
               << "      }\n"
               << "    }";
        if (i + 1 < selected_components.size()) {
            output << ',';
        }
        output << '\n';
    }

    output << "  ]\n"
           << "}\n";
}

double elapsed_milliseconds(const std::chrono::steady_clock::time_point begin) {
    const auto elapsed = std::chrono::steady_clock::now() - begin;
    return std::chrono::duration<double, std::milli>(elapsed).count();
}

void write_lab3_comparison_json(
    const std::string& output_path,
    const std::string& input_path,
    const GrayImage& original,
    const double angle_degrees
) {
    struct MethodResult {
        lab03::InterpolationMethod method;
        double rotation_ms{0.0};
        double inverse_rotation_ms{0.0};
        double mse{0.0};
        double psnr{0.0};
        std::size_t rotated_width{0};
        std::size_t rotated_height{0};
    };

    constexpr std::array<lab03::InterpolationMethod, 3> methods{{
        lab03::InterpolationMethod::nearest,
        lab03::InterpolationMethod::bilinear,
        lab03::InterpolationMethod::bicubic,
    }};

    std::vector<MethodResult> results;
    results.reserve(methods.size());

    for (const lab03::InterpolationMethod method : methods) {
        const auto rotation_begin = std::chrono::steady_clock::now();
        const GrayImage rotated = lab03::rotate_image(original, angle_degrees, method);
        const double rotation_ms = elapsed_milliseconds(rotation_begin);

        const auto inverse_begin = std::chrono::steady_clock::now();
        const GrayImage restored_canvas = lab03::rotate_image(rotated, -angle_degrees, method);
        const double inverse_rotation_ms = elapsed_milliseconds(inverse_begin);

        const GrayImage restored = center_crop_or_pad(restored_canvas, original.width(), original.height());
        results.push_back({
            method,
            rotation_ms,
            inverse_rotation_ms,
            lab01::mean_squared_error(original, restored),
            lab01::peak_signal_to_noise_ratio(original, restored),
            rotated.width(),
            rotated.height(),
        });
    }

    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("failed to open output file: " + output_path);
    }

    output << std::fixed << std::setprecision(12);
    output << "{\n"
           << "  \"input\": \"" << input_path << "\",\n"
           << "  \"angle_degrees\": " << angle_degrees << ",\n"
           << "  \"quality_method\": \"rotate by angle, rotate back by negative angle, center-crop to original size\",\n"
           << "  \"original_width\": " << original.width() << ",\n"
           << "  \"original_height\": " << original.height() << ",\n"
           << "  \"methods\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const MethodResult& result = results[i];
        output << "    {\n"
               << "      \"method\": \"" << lab03::interpolation_method_name(result.method) << "\",\n"
               << "      \"rotated_width\": " << result.rotated_width << ",\n"
               << "      \"rotated_height\": " << result.rotated_height << ",\n"
               << "      \"rotation_ms\": " << result.rotation_ms << ",\n"
               << "      \"inverse_rotation_ms\": " << result.inverse_rotation_ms << ",\n"
               << "      \"roundtrip_mse\": " << result.mse << ",\n"
               << "      \"roundtrip_psnr_db\": ";

        if (std::isinf(result.psnr)) {
            output << "\"inf\"";
        } else {
            output << result.psnr;
        }

        output << "\n"
               << "    }";
        if (i + 1 != results.size()) {
            output << ",";
        }
        output << "\n";
    }

    output << "  ]\n"
           << "}\n";
}

int run_lab1_glcm(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string matrix_output_path;
    int row_offset = 0;
    int column_offset = 0;
    bool has_row_offset = false;
    bool has_column_offset = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--matrix-output" && i + 1 < args.size()) {
            matrix_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--dr" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], row_offset)) {
                std::cerr << "Invalid integer value for --dr: " << args[i + 1] << '\n';
                return 2;
            }
            has_row_offset = true;
            ++i;
        } else if (args[i] == "--dc" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], column_offset)) {
                std::cerr << "Invalid integer value for --dc: " << args[i + 1] << '\n';
                return 2;
            }
            has_column_offset = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab1 glcm: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_row_offset) {
        std::cerr << "Missing required option: --dr <rows>\n";
        return 2;
    }

    if (!has_column_offset) {
        std::cerr << "Missing required option: --dc <columns>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const lab01::CooccurrenceMatrix matrix = lab01::cooccurrence_matrix(image, row_offset, column_offset);
    write_cooccurrence_json(output_path, input_path, image, matrix);

    if (!matrix_output_path.empty()) {
        write_gray_image(matrix_output_path, lab01::render_cooccurrence_image(matrix));
    }

    return 0;
}

int run_lab1_noise(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    double variance = 0.0;
    std::uint32_t seed = 1;
    bool has_variance = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--variance" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], variance)) {
                std::cerr << "Invalid value for --variance: " << args[i + 1] << '\n';
                return 2;
            }
            has_variance = true;
            ++i;
        } else if (args[i] == "--seed" && i + 1 < args.size()) {
            if (!parse_seed(args[i + 1], seed)) {
                std::cerr << "Invalid value for --seed: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab1 noise: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_variance) {
        std::cerr << "Missing required option: --variance <value>\n";
        return 2;
    }

    if (!std::isfinite(variance) || variance < 0.0) {
        std::cerr << "Variance must be a finite non-negative value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const GrayImage noisy_image = lab01::add_gaussian_noise(image, variance, seed);
    write_gray_image(output_path, noisy_image);

    return 0;
}

int run_lab1_psnr(const std::vector<std::string>& args) {
    std::string original_path;
    std::string distorted_path;
    std::string output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--original" && i + 1 < args.size()) {
            original_path = args[i + 1];
            ++i;
        } else if (args[i] == "--distorted" && i + 1 < args.size()) {
            distorted_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab1 psnr: " << args[i] << '\n';
            return 2;
        }
    }

    if (original_path.empty()) {
        std::cerr << "Missing required option: --original <path>\n";
        return 2;
    }

    if (distorted_path.empty()) {
        std::cerr << "Missing required option: --distorted <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const GrayImage original = read_gray_image(original_path);
    const GrayImage distorted = read_gray_image(distorted_path);
    write_psnr_json(output_path, original_path, distorted_path, original, distorted);

    return 0;
}

int run_lab1(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab1");
        return 0;
    }

    if (!args.empty() && args.front() == "stats") {
        return run_lab1_stats({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "glcm") {
        return run_lab1_glcm({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "noise") {
        return run_lab1_noise({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "psnr") {
        return run_lab1_psnr({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab1 command.\n";
    print_lab_help(std::cerr, "lab1");
    return 2;
}

int run_lab2_signal_spectrum(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab2 signal-spectrum: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const std::vector<double> samples = read_signal_samples(input_path);
    const std::vector<lab02::Complex> spectrum = lab02::forward_fft_signal(samples);
    write_gray_image(output_path, lab02::render_signal_log_amplitude_spectrum(spectrum));

    return 0;
}

int run_lab2_signal_plot(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab2 signal-plot: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const std::vector<double> samples = read_signal_samples(input_path);
    write_gray_image(output_path, lab02::render_signal_plot(samples));

    return 0;
}

int run_lab2_image_spectrum(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab2 image-spectrum: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const lab02::FourierImage spectrum = lab02::forward_fft_image(image);
    write_gray_image(output_path, lab02::render_image_log_amplitude_spectrum(spectrum));

    return 0;
}

int run_lab2(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab2");
        return 0;
    }

    if (!args.empty() && args.front() == "signal-spectrum") {
        return run_lab2_signal_spectrum({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "signal-plot") {
        return run_lab2_signal_plot({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "image-spectrum") {
        return run_lab2_image_spectrum({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab2 command.\n";
    print_lab_help(std::cerr, "lab2");
    return 2;
}

int run_lab3_rotate(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    double angle_degrees = 0.0;
    bool has_angle = false;
    lab03::InterpolationMethod method = lab03::InterpolationMethod::bilinear;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--angle" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], angle_degrees)) {
                std::cerr << "Invalid value for --angle: " << args[i + 1] << '\n';
                return 2;
            }
            has_angle = true;
            ++i;
        } else if (args[i] == "--method" && i + 1 < args.size()) {
            if (!parse_interpolation_method(args[i + 1], method)) {
                std::cerr << "Invalid interpolation method for --method: " << args[i + 1] << '\n';
                std::cerr << "Expected one of: nearest, bilinear, bicubic.\n";
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab3 rotate: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_angle) {
        std::cerr << "Missing required option: --angle <degrees>\n";
        return 2;
    }

    if (!std::isfinite(angle_degrees)) {
        std::cerr << "Angle must be a finite value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(output_path, lab03::rotate_image(image, angle_degrees, method));

    return 0;
}

int run_lab3_compare(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    double angle_degrees = 0.0;
    bool has_angle = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--angle" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], angle_degrees)) {
                std::cerr << "Invalid value for --angle: " << args[i + 1] << '\n';
                return 2;
            }
            has_angle = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab3 compare: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_angle) {
        std::cerr << "Missing required option: --angle <degrees>\n";
        return 2;
    }

    if (!std::isfinite(angle_degrees)) {
        std::cerr << "Angle must be a finite value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_lab3_comparison_json(output_path, input_path, image, angle_degrees);

    return 0;
}

int run_lab3(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab3");
        return 0;
    }

    if (!args.empty() && args.front() == "rotate") {
        return run_lab3_rotate({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "compare") {
        return run_lab3_compare({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab3 command.\n";
    print_lab_help(std::cerr, "lab3");
    return 2;
}

int run_lab4_convolve(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string kernel_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel" && i + 1 < args.size()) {
            kernel_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 convolve: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (kernel_path.empty()) {
        std::cerr << "Missing required option: --kernel <path>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const lab04::Kernel2D kernel = read_kernel_file(kernel_path);
    write_gray_image(output_path, lab04::convolve_valid(image, kernel));

    return 0;
}

int run_lab4_threshold_lowpass(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    int kernel_size_value = 0;
    bool has_kernel_size = false;
    double threshold = 0.0;
    bool has_threshold = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            has_kernel_size = true;
            ++i;
        } else if (args[i] == "--threshold" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], threshold)) {
                std::cerr << "Invalid value for --threshold: " << args[i + 1] << '\n';
                return 2;
            }
            has_threshold = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 threshold-lowpass: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_kernel_size) {
        std::cerr << "Missing required option: --kernel-size <odd>\n";
        return 2;
    }

    if (kernel_size_value <= 0 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be a positive odd integer.\n";
        return 2;
    }

    if (!has_threshold) {
        std::cerr << "Missing required option: --threshold <value>\n";
        return 2;
    }

    if (!std::isfinite(threshold) || threshold < 0.0) {
        std::cerr << "Threshold must be a finite non-negative value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(
        output_path,
        lab04::thresholded_average_filter(image, static_cast<std::size_t>(kernel_size_value), threshold)
    );

    return 0;
}

int run_lab4_lowpass_denoise(const std::vector<std::string>& args) {
    std::string input_path;
    std::string noisy_output_path;
    std::string filtered_output_path;
    std::string metrics_output_path;
    NoiseKind noise_kind = NoiseKind::gaussian;
    bool has_noise_kind = false;
    double variance = 0.0;
    bool has_variance = false;
    double probability = 0.0;
    bool has_probability = false;
    int kernel_size_value = 0;
    bool has_kernel_size = false;
    double threshold = 0.0;
    bool has_threshold = false;
    std::uint32_t seed = 1;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--noise" && i + 1 < args.size()) {
            if (!parse_noise_kind(args[i + 1], noise_kind)) {
                std::cerr << "Invalid noise type for --noise: " << args[i + 1] << '\n';
                std::cerr << "Expected one of: gaussian, impulse.\n";
                return 2;
            }
            has_noise_kind = true;
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            has_kernel_size = true;
            ++i;
        } else if (args[i] == "--noisy-output" && i + 1 < args.size()) {
            noisy_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--filtered-output" && i + 1 < args.size()) {
            filtered_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--variance" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], variance)) {
                std::cerr << "Invalid value for --variance: " << args[i + 1] << '\n';
                return 2;
            }
            has_variance = true;
            ++i;
        } else if (args[i] == "--probability" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], probability)) {
                std::cerr << "Invalid value for --probability: " << args[i + 1] << '\n';
                return 2;
            }
            has_probability = true;
            ++i;
        } else if (args[i] == "--threshold" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], threshold)) {
                std::cerr << "Invalid value for --threshold: " << args[i + 1] << '\n';
                return 2;
            }
            has_threshold = true;
            ++i;
        } else if (args[i] == "--seed" && i + 1 < args.size()) {
            if (!parse_seed(args[i + 1], seed)) {
                std::cerr << "Invalid value for --seed: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 lowpass-denoise: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (!has_noise_kind) {
        std::cerr << "Missing required option: --noise <gaussian|impulse>\n";
        return 2;
    }

    if (!has_kernel_size) {
        std::cerr << "Missing required option: --kernel-size <odd>\n";
        return 2;
    }

    if (kernel_size_value <= 0 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be a positive odd integer.\n";
        return 2;
    }

    if (noisy_output_path.empty()) {
        std::cerr << "Missing required option: --noisy-output <path>\n";
        return 2;
    }

    if (filtered_output_path.empty()) {
        std::cerr << "Missing required option: --filtered-output <path>\n";
        return 2;
    }

    if (metrics_output_path.empty()) {
        std::cerr << "Missing required option: --metrics-output <path>\n";
        return 2;
    }

    if (has_threshold && (!std::isfinite(threshold) || threshold < 0.0)) {
        std::cerr << "Threshold must be a finite non-negative value.\n";
        return 2;
    }

    const GrayImage original = read_gray_image(input_path);
    GrayImage noisy;
    std::string noise_parameter_name;
    double noise_parameter_value = 0.0;

    if (noise_kind == NoiseKind::gaussian) {
        if (!has_variance) {
            std::cerr << "Missing required option for gaussian noise: --variance <value>\n";
            return 2;
        }

        if (!std::isfinite(variance) || variance < 0.0) {
            std::cerr << "Variance must be a finite non-negative value.\n";
            return 2;
        }

        noisy = lab01::add_gaussian_noise(original, variance, seed);
        noise_parameter_name = "variance";
        noise_parameter_value = variance;
    } else {
        if (!has_probability) {
            std::cerr << "Missing required option for impulse noise: --probability <value>\n";
            return 2;
        }

        if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0) {
            std::cerr << "Probability must be in the [0, 1] range.\n";
            return 2;
        }

        noisy = lab01::add_impulse_noise(original, probability, seed);
        noise_parameter_name = "probability";
        noise_parameter_value = probability;
    }

    const std::size_t kernel_size = static_cast<std::size_t>(kernel_size_value);
    const GrayImage filtered = has_threshold
        ? lab04::thresholded_average_filter(noisy, kernel_size, threshold)
        : lab04::average_low_pass_filter(noisy, kernel_size);
    const GrayImage original_crop = center_crop_or_pad(original, filtered.width(), filtered.height());
    const GrayImage noisy_crop = center_crop_or_pad(noisy, filtered.width(), filtered.height());

    write_gray_image(noisy_output_path, noisy);
    write_gray_image(filtered_output_path, filtered);
    write_lowpass_denoise_json(
        metrics_output_path,
        input_path,
        noisy_output_path,
        filtered_output_path,
        noise_kind,
        noise_parameter_name,
        noise_parameter_value,
        seed,
        kernel_size,
        has_threshold,
        threshold,
        original,
        original_crop,
        noisy_crop,
        filtered
    );

    return 0;
}

int run_lab4_laplacian(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    lab04::LaplacianKernel kernel = lab04::LaplacianKernel::four_neighbor;
    int kernel_size_value = 3;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel" && i + 1 < args.size()) {
            if (!parse_laplacian_kernel(args[i + 1], kernel)) {
                std::cerr << "Invalid Laplacian kernel for --kernel: " << args[i + 1] << '\n';
                std::cerr << "Expected one of: four, eight.\n";
                return 2;
            }
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 laplacian: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (kernel_size_value < 3 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be an odd integer not less than 3.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(output_path, lab04::laplacian_filter(
        image,
        kernel,
        static_cast<std::size_t>(kernel_size_value)
    ));

    return 0;
}

int run_lab4_log_filter(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    int kernel_size_value = 0;
    bool has_kernel_size = false;
    double sigma = 0.0;
    bool has_sigma = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            has_kernel_size = true;
            ++i;
        } else if (args[i] == "--sigma" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], sigma)) {
                std::cerr << "Invalid value for --sigma: " << args[i + 1] << '\n';
                return 2;
            }
            has_sigma = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 log-filter: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_kernel_size) {
        std::cerr << "Missing required option: --kernel-size <odd>\n";
        return 2;
    }

    if (kernel_size_value <= 0 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be a positive odd integer.\n";
        return 2;
    }

    if (!has_sigma) {
        std::cerr << "Missing required option: --sigma <value>\n";
        return 2;
    }

    if (!std::isfinite(sigma) || sigma <= 0.0) {
        std::cerr << "Sigma must be a finite positive value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(
        output_path,
        lab04::log_filter(image, static_cast<std::size_t>(kernel_size_value), sigma)
    );

    return 0;
}

int run_lab4_zero_crossing(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string metrics_output_path;
    int kernel_size_value = 0;
    bool has_kernel_size = false;
    double sigma = 0.0;
    bool has_sigma = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            has_kernel_size = true;
            ++i;
        } else if (args[i] == "--sigma" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], sigma)) {
                std::cerr << "Invalid value for --sigma: " << args[i + 1] << '\n';
                return 2;
            }
            has_sigma = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 zero-crossing: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (!has_kernel_size) {
        std::cerr << "Missing required option: --kernel-size <odd>\n";
        return 2;
    }

    if (kernel_size_value <= 0 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be a positive odd integer.\n";
        return 2;
    }

    if (!has_sigma) {
        std::cerr << "Missing required option: --sigma <value>\n";
        return 2;
    }

    if (!std::isfinite(sigma) || sigma <= 0.0) {
        std::cerr << "Sigma must be a finite positive value.\n";
        return 2;
    }

    const std::size_t kernel_size = static_cast<std::size_t>(kernel_size_value);
    const GrayImage image = read_gray_image(input_path);
    const lab04::ConvolutionResponse response = lab04::log_response(image, kernel_size, sigma);
    const double threshold = lab04::automatic_zero_crossing_threshold(response);
    const GrayImage edges = lab04::zero_crossing_edges(response, threshold);

    write_gray_image(output_path, edges);

    if (!metrics_output_path.empty()) {
        write_zero_crossing_json(
            metrics_output_path,
            input_path,
            output_path,
            image,
            edges,
            kernel_size,
            sigma,
            threshold
        );
    }

    return 0;
}

int run_lab4_sharpen(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    int kernel_size_value = 5;
    double amount = 1.0;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--kernel-size" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], kernel_size_value)) {
                std::cerr << "Invalid integer value for --kernel-size: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else if (args[i] == "--amount" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], amount)) {
                std::cerr << "Invalid value for --amount: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab4 sharpen: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (kernel_size_value < 3 || kernel_size_value % 2 == 0) {
        std::cerr << "Kernel size must be an odd integer not less than 3.\n";
        return 2;
    }

    if (!std::isfinite(amount) || amount < 0.0) {
        std::cerr << "Amount must be a finite non-negative value.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(output_path, lab04::sharpening_filter(
        image,
        static_cast<std::size_t>(kernel_size_value),
        amount
    ));

    return 0;
}

int run_lab4(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab4");
        return 0;
    }

    if (!args.empty() && args.front() == "convolve") {
        return run_lab4_convolve({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "threshold-lowpass") {
        return run_lab4_threshold_lowpass({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "lowpass-denoise") {
        return run_lab4_lowpass_denoise({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "laplacian") {
        return run_lab4_laplacian({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "log-filter") {
        return run_lab4_log_filter({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "zero-crossing") {
        return run_lab4_zero_crossing({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "sharpen") {
        return run_lab4_sharpen({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab4 command.\n";
    print_lab_help(std::cerr, "lab4");
    return 2;
}

int run_lab5_rank(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string aperture_path;
    std::string rank_text;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--aperture" && i + 1 < args.size()) {
            aperture_path = args[i + 1];
            ++i;
        } else if (args[i] == "--rank" && i + 1 < args.size()) {
            rank_text = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab5 rank: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (aperture_path.empty()) {
        std::cerr << "Missing required option: --aperture <path>\n";
        return 2;
    }

    if (rank_text.empty()) {
        std::cerr << "Missing required option: --rank <index|min|median|max>\n";
        return 2;
    }

    const lab05::Aperture aperture = read_aperture_file(aperture_path);
    const std::size_t active_size = lab05::active_aperture_size(aperture);
    std::size_t rank = 0;
    if (!parse_rank(rank_text, active_size, rank)) {
        std::cerr << "Rank must be min, median, max, or an integer in [0, "
                  << (active_size == 0 ? 0 : active_size - 1) << "].\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(output_path, lab05::rank_filter_valid(image, aperture, rank));

    return 0;
}

int run_lab5_trimmed_mean(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string aperture_path;
    int trimmed_count_value = 0;
    bool has_trimmed_count = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--aperture" && i + 1 < args.size()) {
            aperture_path = args[i + 1];
            ++i;
        } else if (args[i] == "--trimmed-count" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], trimmed_count_value)) {
                std::cerr << "Invalid integer value for --trimmed-count: " << args[i + 1] << '\n';
                return 2;
            }
            has_trimmed_count = true;
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab5 trimmed-mean: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (aperture_path.empty()) {
        std::cerr << "Missing required option: --aperture <path>\n";
        return 2;
    }

    if (!has_trimmed_count) {
        std::cerr << "Missing required option: --trimmed-count <even>\n";
        return 2;
    }

    if (trimmed_count_value < 0) {
        std::cerr << "Trimmed count must be non-negative.\n";
        return 2;
    }

    const lab05::Aperture aperture = read_aperture_file(aperture_path);
    const std::size_t active_size = lab05::active_aperture_size(aperture);
    const std::size_t trimmed_count = static_cast<std::size_t>(trimmed_count_value);
    if (trimmed_count >= active_size || trimmed_count % 2 != 0) {
        std::cerr << "Trimmed count must be even and in [0, "
                  << (active_size == 0 ? 0 : active_size - 1) << "].\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    write_gray_image(output_path, lab05::trimmed_mean_filter_valid(image, aperture, trimmed_count));

    return 0;
}

int run_lab5_morphology(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string aperture_path;
    std::string operation_text;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--aperture" && i + 1 < args.size()) {
            aperture_path = args[i + 1];
            ++i;
        } else if (args[i] == "--operation" && i + 1 < args.size()) {
            operation_text = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab5 morphology: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    if (aperture_path.empty()) {
        std::cerr << "Missing required option: --aperture <path>\n";
        return 2;
    }

    if (operation_text.empty()) {
        std::cerr << "Missing required option: --operation <erosion|dilation|opening|closing>\n";
        return 2;
    }

    MorphologyOperation operation = MorphologyOperation::erosion;
    if (!parse_morphology_operation(operation_text, operation)) {
        std::cerr << "Operation must be one of: erosion, dilation, opening, closing.\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    const lab05::Aperture aperture = read_aperture_file(aperture_path);

    GrayImage result;
    switch (operation) {
        case MorphologyOperation::erosion:
            result = lab05::erosion_valid(image, aperture);
            break;
        case MorphologyOperation::dilation:
            result = lab05::dilation_valid(image, aperture);
            break;
        case MorphologyOperation::opening:
            result = lab05::opening_valid(image, aperture);
            break;
        case MorphologyOperation::closing:
            result = lab05::closing_valid(image, aperture);
            break;
    }

    write_gray_image(output_path, result);

    return 0;
}

int run_lab5_compare_denoise(const std::vector<std::string>& args) {
    std::string input_path;
    std::string aperture_path;
    std::string noisy_output_path;
    std::string median_output_path;
    std::string trimmed_output_path;
    std::string metrics_output_path;
    NoiseKind noise_kind = NoiseKind::gaussian;
    bool has_noise_kind = false;
    double variance = 0.0;
    bool has_variance = false;
    double probability = 0.0;
    bool has_probability = false;
    int trimmed_count_value = 0;
    bool has_trimmed_count = false;
    std::uint32_t seed = 1;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--noise" && i + 1 < args.size()) {
            if (!parse_noise_kind(args[i + 1], noise_kind)) {
                std::cerr << "Invalid noise type for --noise: " << args[i + 1] << '\n';
                std::cerr << "Expected one of: gaussian, impulse.\n";
                return 2;
            }
            has_noise_kind = true;
            ++i;
        } else if (args[i] == "--aperture" && i + 1 < args.size()) {
            aperture_path = args[i + 1];
            ++i;
        } else if (args[i] == "--trimmed-count" && i + 1 < args.size()) {
            if (!parse_int(args[i + 1], trimmed_count_value)) {
                std::cerr << "Invalid integer value for --trimmed-count: " << args[i + 1] << '\n';
                return 2;
            }
            has_trimmed_count = true;
            ++i;
        } else if (args[i] == "--noisy-output" && i + 1 < args.size()) {
            noisy_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--median-output" && i + 1 < args.size()) {
            median_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--trimmed-output" && i + 1 < args.size()) {
            trimmed_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--variance" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], variance)) {
                std::cerr << "Invalid value for --variance: " << args[i + 1] << '\n';
                return 2;
            }
            has_variance = true;
            ++i;
        } else if (args[i] == "--probability" && i + 1 < args.size()) {
            if (!parse_double(args[i + 1], probability)) {
                std::cerr << "Invalid value for --probability: " << args[i + 1] << '\n';
                return 2;
            }
            has_probability = true;
            ++i;
        } else if (args[i] == "--seed" && i + 1 < args.size()) {
            if (!parse_seed(args[i + 1], seed)) {
                std::cerr << "Invalid value for --seed: " << args[i + 1] << '\n';
                return 2;
            }
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab5 compare-denoise: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (!has_noise_kind) {
        std::cerr << "Missing required option: --noise <gaussian|impulse>\n";
        return 2;
    }

    if (aperture_path.empty()) {
        std::cerr << "Missing required option: --aperture <path>\n";
        return 2;
    }

    if (!has_trimmed_count) {
        std::cerr << "Missing required option: --trimmed-count <even>\n";
        return 2;
    }

    if (trimmed_count_value < 0) {
        std::cerr << "Trimmed count must be non-negative.\n";
        return 2;
    }

    if (noisy_output_path.empty()) {
        std::cerr << "Missing required option: --noisy-output <path>\n";
        return 2;
    }

    if (median_output_path.empty()) {
        std::cerr << "Missing required option: --median-output <path>\n";
        return 2;
    }

    if (trimmed_output_path.empty()) {
        std::cerr << "Missing required option: --trimmed-output <path>\n";
        return 2;
    }

    if (metrics_output_path.empty()) {
        std::cerr << "Missing required option: --metrics-output <path>\n";
        return 2;
    }

    const GrayImage original = read_gray_image(input_path);
    GrayImage noisy;
    std::string noise_parameter_name;
    double noise_parameter_value = 0.0;

    if (noise_kind == NoiseKind::gaussian) {
        if (!has_variance) {
            std::cerr << "Missing required option for gaussian noise: --variance <value>\n";
            return 2;
        }

        if (!std::isfinite(variance) || variance < 0.0) {
            std::cerr << "Variance must be a finite non-negative value.\n";
            return 2;
        }

        noisy = lab01::add_gaussian_noise(original, variance, seed);
        noise_parameter_name = "variance";
        noise_parameter_value = variance;
    } else {
        if (!has_probability) {
            std::cerr << "Missing required option for impulse noise: --probability <value>\n";
            return 2;
        }

        if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0) {
            std::cerr << "Probability must be in the [0, 1] range.\n";
            return 2;
        }

        noisy = lab01::add_impulse_noise(original, probability, seed);
        noise_parameter_name = "probability";
        noise_parameter_value = probability;
    }

    const lab05::Aperture aperture = read_aperture_file(aperture_path);
    const std::size_t active_size = lab05::active_aperture_size(aperture);
    const std::size_t trimmed_count = static_cast<std::size_t>(trimmed_count_value);
    if (trimmed_count >= active_size || trimmed_count % 2 != 0) {
        std::cerr << "Trimmed count must be even and in [0, "
                  << (active_size == 0 ? 0 : active_size - 1) << "].\n";
        return 2;
    }

    const std::size_t median_rank = active_size / 2;
    const GrayImage median_filtered = lab05::rank_filter_valid(noisy, aperture, median_rank);
    const GrayImage trimmed_filtered = lab05::trimmed_mean_filter_valid(noisy, aperture, trimmed_count);
    const GrayImage original_crop = center_crop_or_pad(original, median_filtered.width(), median_filtered.height());
    const GrayImage noisy_crop = center_crop_or_pad(noisy, median_filtered.width(), median_filtered.height());

    write_gray_image(noisy_output_path, noisy);
    write_gray_image(median_output_path, median_filtered);
    write_gray_image(trimmed_output_path, trimmed_filtered);
    write_lab5_denoise_comparison_json(
        metrics_output_path,
        input_path,
        noisy_output_path,
        median_output_path,
        trimmed_output_path,
        noise_kind,
        noise_parameter_name,
        noise_parameter_value,
        seed,
        aperture,
        median_rank,
        trimmed_count,
        original,
        original_crop,
        noisy_crop,
        median_filtered,
        trimmed_filtered
    );

    return 0;
}

int run_lab5(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab5");
        return 0;
    }

    if (!args.empty() && args.front() == "rank") {
        return run_lab5_rank({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "trimmed-mean") {
        return run_lab5_trimmed_mean({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "morphology") {
        return run_lab5_morphology({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "compare-denoise") {
        return run_lab5_compare_denoise({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab5 command.\n";
    print_lab_help(std::cerr, "lab5");
    return 2;
}

int run_lab6_otsu(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string metrics_output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab6 otsu: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const GrayImage image = read_gray_image(input_path);
    lab06::OtsuResult result;
    const GrayImage binary = lab06::otsu_binarize(image, result);

    write_gray_image(output_path, binary);
    if (!metrics_output_path.empty()) {
        write_otsu_json(metrics_output_path, input_path, output_path, image, result);
    }

    return 0;
}

int run_lab6_components(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string metrics_output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab6 components: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    const GrayImage binary = read_gray_image(input_path);
    const lab06::ConnectedComponentsResult result = lab06::label_4_connected_components(binary);
    const RgbImage labels = lab06::render_component_labels_color(result);

    write_rgb_image(output_path, labels);
    if (!metrics_output_path.empty()) {
        write_connected_components_json(metrics_output_path, input_path, output_path, result);
    }

    return 0;
}

int run_lab6_circles(const std::vector<std::string>& args) {
    std::string input_path;
    std::string output_path;
    std::string metrics_output_path;

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--input" && i + 1 < args.size()) {
            input_path = args[i + 1];
            ++i;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            output_path = args[i + 1];
            ++i;
        } else if (args[i] == "--metrics-output" && i + 1 < args.size()) {
            metrics_output_path = args[i + 1];
            ++i;
        } else {
            std::cerr << "Unknown or incomplete option for lab6 circles: " << args[i] << '\n';
            return 2;
        }
    }

    if (input_path.empty()) {
        std::cerr << "Missing required option: --input <path>\n";
        return 2;
    }

    if (output_path.empty()) {
        std::cerr << "Missing required option: --output <path>\n";
        return 2;
    }

    constexpr std::size_t min_area = 30;
    constexpr double max_roundness_deviation = 0.02;

    const GrayImage binary = read_gray_image(input_path);
    const lab06::ConnectedComponentsResult result = lab06::label_4_connected_components(binary);
    const std::vector<lab06::CircleLikeComponent> selected = lab06::find_large_circle_like_components(
        result,
        min_area,
        max_roundness_deviation
    );
    const RgbImage highlighted = lab06::render_selected_components_color(result, selected);

    write_rgb_image(output_path, highlighted);
    if (!metrics_output_path.empty()) {
        write_circle_like_components_json(
            metrics_output_path,
            input_path,
            output_path,
            result,
            selected,
            min_area,
            max_roundness_deviation
        );
    }

    return 0;
}

int run_lab6(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab6");
        return 0;
    }

    if (!args.empty() && args.front() == "otsu") {
        return run_lab6_otsu({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "components") {
        return run_lab6_components({args.begin() + 1, args.end()});
    }

    if (!args.empty() && args.front() == "circles") {
        return run_lab6_circles({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab6 command.\n";
    print_lab_help(std::cerr, "lab6");
    return 2;
}

} // namespace

int run_cli(const int argc, char** argv) {
    if (argc < 2) {
        print_general_help(std::cerr);
        return 2;
    }

    const std::string command = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    try {
        if (command == "--help" || command == "-h") {
            print_general_help(std::cout);
            return 0;
        }

        if (command == "info") {
            return run_info(args);
        }

        if (command == "lab1") {
            return run_lab1(args);
        }

        if (command == "lab2") {
            return run_lab2(args);
        }

        if (command == "lab3") {
            return run_lab3(args);
        }

        if (command == "lab4") {
            return run_lab4(args);
        }

        if (command == "lab5") {
            return run_lab5(args);
        }

        if (command == "lab6") {
            return run_lab6(args);
        }

        if (is_lab_command(command)) {
            if (args.size() == 1 && args.front() == "--help") {
                print_lab_help(std::cout, command);
                return 0;
            }

            std::cerr << "Only --help is available for " << command << ".\n";
            return 2;
        }

        std::cerr << "Unknown command: " << command << '\n';
        print_general_help(std::cerr);
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

} // namespace dip
