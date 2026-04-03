#pragma once

#include <vector>
#include <memory>

namespace jpeg_decoder {
    class DctCalculator {
    public:
        DctCalculator(size_t width, std::vector<double> *input, std::vector<double> *output);

        void Inverse() const;

        ~DctCalculator();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
