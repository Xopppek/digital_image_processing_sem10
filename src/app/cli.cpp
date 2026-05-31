#include "app/cli.hpp"

#include "core/image_io.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace dip {
namespace {

void print_general_help(std::ostream& output) {
    output << "Usage:\n"
           << "  dip info --input <path>\n"
           << "  dip lab1 --help\n"
           << "  dip lab2 --help\n"
           << "  dip lab3 --help\n"
           << "  dip lab4 --help\n"
           << "  dip lab5 --help\n"
           << "  dip lab6 --help\n";
}

void print_lab_help(std::ostream& output, const std::string& lab) {
    output << "Usage:\n"
           << "  dip " << lab << " --help\n\n"
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
