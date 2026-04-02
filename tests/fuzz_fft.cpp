#include <cmath>

#include <fft.h>

#include <cstdint>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 64 * sizeof(double)) {
        return 0;
    }
    std::vector<double> input(64);
    std::vector<double> output(64);
    DctCalculator calculator(8, &input, &output);
    std::memcpy(input.data(), data, 64 * sizeof(double));
    for (double& x : input) {
        if (!std::isfinite(x)) {
            x = 0.0;
        }
    }
    calculator.Inverse();
    return 0;
}
