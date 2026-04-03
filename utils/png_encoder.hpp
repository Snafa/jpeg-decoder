#pragma once

#include <string>

#include "../include/jpeg_decoder/image.h"

void WritePng(const std::string& filename, const jpeg_decoder::Image& image);
