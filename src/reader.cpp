#include "reader.h"

#include <istream>
#include <stdexcept>

namespace jpeg_decoder {
    Reader::Reader(std::istream &input)
        : input_(&input), bytes_in_buffer_(0), position_byte_(0), position_bit_(0), current_byte_(0) {
        ReadBuffer();
    }

    Reader::~Reader() = default;

    void Reader::ReadBuffer() {
        if (input_ == nullptr) {
            throw std::runtime_error("Reader: null input stream");
        }

        input_->read(reinterpret_cast<char *>(buffer_), SizeOfBuffer);
        bytes_in_buffer_ = static_cast<size_t>(input_->gcount());
        position_byte_ = 0;
    }

    uint8_t Reader::ReadBit() {
        auto read_raw_byte = [this]() -> uint8_t {
            if (position_byte_ >= bytes_in_buffer_) {
                ReadBuffer();
                if (bytes_in_buffer_ == 0) {
                    throw std::runtime_error("Reader: unexpected end of stream");
                }
            }
            return buffer_[position_byte_++];
        };

        if (position_bit_ == 0) {
            current_byte_ = read_raw_byte();

            if (current_byte_ == 0xFF) {
                if (const uint8_t next = read_raw_byte(); next == 0x00) {
                    current_byte_ = 0xFF;
                } else {
                    throw std::runtime_error("Reader: marker inside entropy-coded data ");
                }
            }

            position_bit_ = 8;
        }

        --position_bit_;
        return static_cast<uint8_t>(current_byte_ >> position_bit_ & 1u);
    }

    uint8_t Reader::ReadByte() {
        if (position_bit_ != 0) {
            position_bit_ = 0;
        }

        if (position_byte_ >= bytes_in_buffer_) {
            ReadBuffer();
            if (bytes_in_buffer_ == 0) {
                throw std::runtime_error("Reader: unexpected end of stream");
            }
        }

        return buffer_[position_byte_++];
    }

    uint16_t Reader::Read2Bytes() {
        const uint16_t hi = ReadByte();
        const uint16_t lo = ReadByte();
        return static_cast<uint16_t>(hi << 8u | lo);
    }

    uint16_t Reader::ReadMarker() {
        if (position_bit_ != 0) {
            position_bit_ = 0;
        }

        uint8_t first = 0;
        do {
            first = ReadByte();
        } while (first != 0xFF);

        uint8_t second = 0;
        do {
            second = ReadByte();
        } while (second == 0xFF);

        return static_cast<uint16_t>(static_cast<uint16_t>(first) << 8u | second);
    }

    bool Reader::IsEnd() {
        if (position_byte_ < bytes_in_buffer_) {
            return false;
        }

        if (input_ == nullptr) {
            return true;
        }

        if (input_->eof()) {
            return true;
        }

        ReadBuffer();
        return bytes_in_buffer_ == 0;
    }
}
