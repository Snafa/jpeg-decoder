#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

namespace jpeg_decoder {
    struct RGB {
        std::uint8_t r, g, b;
    };

    class Image {
    public:
        Image() = default;

        Image(const size_t width, const size_t height) {
            SetSize(width, height);
        }

        void SetSize(const size_t width, const size_t height) {
#ifdef MAX_ALLOWED_IMAGE_SIZE_BYTES
            if (width * height * sizeof(RGB) > MAX_ALLOWED_IMAGE_SIZE_BYTES) {
                throw std::invalid_argument("Too big image");
            }
#endif
            width_ = width;
            height_ = height;
            data_.assign(width_ * height_, {});
        }

        [[nodiscard]] size_t Width() const {
            return width_;
        }

        [[nodiscard]] size_t Height() const {
            return height_;
        }

        [[nodiscard]] RGB Pixel(const size_t y, const size_t x) const {
            return data_[y * width_ + x];
        }

        RGB &Pixel(const size_t y, const size_t x) {
            return data_[y * width_ + x];
        }

        std::string &Comment() {
            return comment_;
        }

        [[nodiscard]] const std::string &Comment() const {
            return comment_;
        }

    private:
        std::size_t width_ = 0;
        std::size_t height_ = 0;
        std::vector<RGB> data_;
        std::string comment_;
    };
} // namespace jpeg_decoder
