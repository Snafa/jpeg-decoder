#include <../include/jpeg_decoder/decoder.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#ifndef HSE_TASK_DIR
#error HSE_TASK_DIR must be defined
#endif

namespace fs = std::filesystem;

int main() {
    try {
        const fs::path base = fs::path(HSE_TASK_DIR);
        const fs::path dir = base / "fuzz" / "artifacts" / "jpeg";

        if (!fs::exists(dir)) {
            std::cerr << "Artifacts directory does not exist: " << dir << '\n';
            return 1;
        }

        std::vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());

        if (files.empty()) {
            std::cerr << "No artifact files found in: " << dir << '\n';
            return 1;
        }

        size_t processed = 0;
        for (const auto& path : files) {
            std::cerr << "Replaying: " << path.filename().string() << '\n';

            std::ifstream fin(path, std::ios::binary);
            if (!fin.is_open()) {
                std::cerr << "Cannot open file: " << path << '\n';
                return 1;
            }

            try {
                [[maybe_unused]] Image image = Decode(fin);
            } catch (const std::exception& e) {
                std::cerr << "  handled with exception: " << e.what() << '\n';
            }

            ++processed;
        }

        std::cout << "Processed artifact files: " << processed << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Replay failed: " << e.what() << '\n';
        return 1;
    }
}