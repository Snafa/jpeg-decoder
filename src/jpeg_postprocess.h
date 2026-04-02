#pragma once

#include <jpeg_types.h>
#include <image.h>

void Inverse(Context&);
std::vector<std::vector<YCbCr>> UpSampling(const Context&);
Image ConvertYCbCrToRgb(const std::vector<std::vector<YCbCr>>&);