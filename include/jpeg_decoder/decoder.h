#pragma once

#include <jpeg_decoder/image.h>
#include <istream>

namespace jpeg_decoder {
    class Image;

    Image Decode(std::istream &input);

    Image DecodeFile(const std::string &path);
} // namespace jpeg_decoder
