#include <jpeg_decoder/decoder.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using jpeg_decoder::Decode;
using jpeg_decoder::Image;

namespace {

void WritePpm(const std::string& path, const Image& image) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open output file: " + path);
    }

    out << "P6\n" << image.Width() << ' ' << image.Height() << "\n255\n";

    for (int y = 0; y < image.Height(); ++y) {
        for (int x = 0; x < image.Width(); ++x) {
            const auto [r, g, b] = image.Pixel(y, x);

            const unsigned char rgb[3] = {
                static_cast<unsigned char>(r),
                static_cast<unsigned char>(g),
                static_cast<unsigned char>(b)
            };
            out.write(reinterpret_cast<const char*>(rgb), 3);
        }
    }

    if (!out) {
        throw std::runtime_error("Failed to write output file: " + path);
    }
}

void PrintUsage(const char* program_name) {
    std::cerr
        << "Usage: " << program_name << " <input.jpg> <output.ppm>\n"
        << "Example: " << program_name << " tests/data/small.jpg out.ppm\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            PrintUsage(argv[0]);
            return 1;
        }

        const std::string input_path = argv[1];
        const std::string output_path = argv[2];

        std::ifstream input(input_path, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "Cannot open input file: " << input_path << '\n';
            return 1;
        }

        Image image = Decode(input);
        WritePpm(output_path, image);

        std::cout
            << "Decoded successfully\n"
            << "Size: " << image.Width() << "x" << image.Height() << '\n';

        if (!image.Comment().empty()) {
            std::cout << "Comment: " << image.Comment() << '\n';
        }

        std::cout << "Saved to: " << output_path << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Decode failed: " << e.what() << '\n';
        return 1;
    }
}