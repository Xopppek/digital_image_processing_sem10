#include "app/cli.hpp"

#include "core/image_io.hpp"
#include "lab01_analysis/cooccurrence.hpp"
#include "lab01_analysis/noise.hpp"
#include "lab01_analysis/quality.hpp"
#include "lab01_analysis/statistics.hpp"
#include "lab02_fourier/fft.hpp"

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
