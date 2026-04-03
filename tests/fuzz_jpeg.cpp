#include <jpeg_decoder/decoder.h>

#include <cstdint>
#include <sstream>
#include <string>

using jpeg_decoder::Decode;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, const size_t size) {
    const std::string s(reinterpret_cast<const char *>(data), size);
    std::stringstream ss(s);
    try {
        const auto image = Decode(ss);
        (void)image;
    } catch (...) {
    }
    return 0;
}
