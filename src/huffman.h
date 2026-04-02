#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <memory>

class HuffmanTree {
public:
    HuffmanTree();

    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    HuffmanTree(HuffmanTree&&);
    HuffmanTree& operator=(HuffmanTree&&);

    void Build(const std::vector<uint8_t>& code_lengths, const std::vector<uint8_t>& values);

    bool Move(bool bit, int& value);

    ~HuffmanTree();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
