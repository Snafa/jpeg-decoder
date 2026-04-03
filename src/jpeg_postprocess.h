#pragma once

#include <jpeg_types.h>
#include <jpeg_decoder/image.h>

namespace jpeg_decoder {
    void Inverse(Context &);

    std::vector<std::vector<YCbCr> > UpSampling(const Context &);

    Image ConvertYCbCrToRgb(const std::vector<std::vector<YCbCr> > &);
}
