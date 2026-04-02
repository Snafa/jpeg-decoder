#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <huffman.h>
#include <image.h>

inline constexpr size_t kTableSize = 64;

class Table {
public:
    int &operator()(const size_t &i, const size_t &j) {
        return table_[i * 8 + j];
    }

    int &operator()(const size_t &i) {
        return table_[kZigZag[i]];
    }

    const int &operator()(const size_t &i, const size_t &j) const {
        return table_[i * 8 + j];
    }

    const int &operator()(const size_t &i) const {
        return table_[kZigZag[i]];
    }

    const Table &operator*=(const Table &table) {
        if (this != &table) {
            for (size_t i = 0; i < kTableSize; ++i) {
                table_[i] *= table.table_[i];
            }
        }
        return *this;
    }

    Table &operator/=(const Table &table) {
        if (this != &table) {
            for (size_t i = 0; i < kTableSize; ++i) {
                table_[i] /= table.table_[i];
            }
        }
        return *this;
    }

private:
    int table_[kTableSize]{};

    static constexpr size_t kZigZag[kTableSize] = {
        0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
        12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
    };
};

enum class TypeCompression { none, horizontal, vertical, all };

struct DecodedBlock {
    uint8_t component_id;
    Table table;
};

struct MCU {
    size_t id;
    TypeCompression type_compression;
    std::vector<DecodedBlock> blocks;
};

struct MCUComponentInfo {
    uint8_t h;
    uint8_t v;
    uint8_t quant_table_id;
};

struct ComponentInfo {
    uint8_t dc_table_id;
    uint8_t ac_table_id;
};

struct Context {
    bool quant_defined[4]{};
    uint8_t max_h = 0;
    uint8_t max_v = 0;
    uint8_t precision = 0;
    uint8_t num_channels = 0;
    uint8_t num_components = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    size_t mcu_width = 0;
    size_t mcu_height = 0;
    size_t mcus_x = 0;
    size_t mcus_y = 0;
    size_t total_mcus = 0;
    Table quant_table[4];
    std::string comment;
    std::vector<MCU> mcu;
    std::vector<uint8_t> scan_order;
    std::array<ComponentInfo, 4> components{};
    std::array<MCUComponentInfo, 4> mcu_components{};
    HuffmanTree huffman_trees[2][4];
    bool huffman_defined[2][4]{};
};

inline int Clamp(double x) {
    if (x < 0) {
        x = 0;
    }
    if (255 < x) {
        x = 255;
    }
    return std::lround(x);
}

struct YCbCr {
    int y, cb, cr;

    explicit operator RGB() const {
        const int r = Clamp(y + ((91881 * (cr - 128)) >> 16));
        const int g = Clamp(y - ((22554 * (cb - 128) + 46802 * (cr - 128)) >> 16));
        const int b = Clamp(y + ((116130 * (cb - 128)) >> 16));
        return {r, g, b};
    }
};