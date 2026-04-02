#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#define SIZE_OF_BUFFER 4096

class Reader {
public:
    explicit Reader(std::istream& input);

    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;

    Reader(Reader&&) = delete;
    Reader& operator=(Reader&&) = delete;

    ~Reader();

    uint8_t ReadBit();

    uint8_t ReadByte();

    uint16_t Read2Bytes();

    uint16_t ReadMarker();

    bool IsEnd();

private:
    std::istream* input_;

    std::uint8_t buffer_[SIZE_OF_BUFFER];
    std::size_t bytes_in_buffer_;
    std::size_t position_byte_;

    std::size_t position_bit_;
    std::uint8_t current_byte_;

    void ReadBuffer();
};