#include "app/cli.hpp"

#include "core/image_io.hpp"
#include "lab01_analysis/statistics.hpp"

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace dip {
namespace {

void print_general_help(std::ostream& output) {
    output << "Usage:\n"
           << "  dip info --input <path>\n"
           << "  dip lab1 stats --input <path> --output <path> [--histogram-output <path>]\n"
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
               << "  dip lab1 --help\n\n"
               << "Available commands:\n"
               << "  stats  Calculate grayscale image statistics and optionally save a histogram image.\n";
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

int run_lab1(const std::vector<std::string>& args) {
    if (args.size() == 1 && args.front() == "--help") {
        print_lab_help(std::cout, "lab1");
        return 0;
    }

    if (!args.empty() && args.front() == "stats") {
        return run_lab1_stats({args.begin() + 1, args.end()});
    }

    std::cerr << "Unknown lab1 command.\n";
    print_lab_help(std::cerr, "lab1");
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
