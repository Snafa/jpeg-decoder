#pragma once

#include <vector>
#include <cstdint>
#include <memory>

namespace jpeg_decoder {
    class HuffmanTree {
    public:
        HuffmanTree();

        HuffmanTree(const HuffmanTree &) = delete;

        HuffmanTree &operator=(const HuffmanTree &) = delete;

        HuffmanTree(HuffmanTree &&) noexcept;

        HuffmanTree &operator=(HuffmanTree &&) noexcept;

        void Build(const std::vector<uint8_t> &code_lengths, const std::vector<uint8_t> &values);

        bool Move(bool bit, int &value);

        ~HuffmanTree();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
