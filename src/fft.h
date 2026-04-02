#pragma once

#include <cstddef>
#include <vector>
#include <memory>

class DctCalculator {
public:
    DctCalculator(size_t width, std::vector<double> *input, std::vector<double> *output);

    void Inverse();

    ~DctCalculator();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
