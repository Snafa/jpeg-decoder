#pragma once

#include <string>

#include "../include/jpeg_decoder/image.h"

jpeg_decoder::Image ReadJpg(const std::string& filename);
