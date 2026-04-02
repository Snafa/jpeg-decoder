#pragma once

#include <vector>
#include <string>
#include <stdexcept>

struct RGB {
    int r, g, b;
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
        data_.assign(height, std::vector<RGB>(width));
    }

    [[nodiscard]] size_t Width() const {
        if (data_.empty()) {
            return 0;
        }
        return data_[0].size();
    }

    [[nodiscard]] size_t Height() const {
        return data_.size();
    }

    void SetPixel(const int y, const int x, const RGB& pixel) {
        data_[y][x] = pixel;
    }

    [[nodiscard]] RGB GetPixel(const int y, const int x) const {
        return data_[y][x];
    }

    RGB& GetPixel(const int y, const int x) {
        return data_[y][x];
    }

    void SetComment(const std::string& comment) {
        comment_ = comment;
    }

    [[nodiscard]] const std::string& GetComment() const {
        return comment_;
    }

private:
    std::vector<std::vector<RGB>> data_;
    std::string comment_;
};
