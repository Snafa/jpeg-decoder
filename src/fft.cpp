#include <fft.h>
#include <require.h>

#include <fftw3.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace jpeg_decoder {
    class DctCalculator::Impl {
    public:
        Impl(const int width, std::vector<double> *input, std::vector<double> *output)
            : width_(width), input_(input), output_(output), scratch_(width * width) {
            Validate();
            RebuildPlan();
        }

        Impl(const Impl &) = delete;

        Impl &operator=(const Impl &) = delete;

        ~Impl() {
            if (plan_ != nullptr) {
                fftw_destroy_plan(plan_);
            }
        }

        void Inverse() {
            Validate();

            if (planned_output_ != output_->data()) {
                RebuildPlan();
            }

            constexpr double k_base = 1.0 / 16.0;
            const double k_sqrt2 = std::sqrt(2.0);

            for (size_t u = 0; u < width_; ++u) {
                const double su = u == 0 ? k_sqrt2 : 1.0;
                for (size_t v = 0; v < width_; ++v) {
                    const double sv = v == 0 ? k_sqrt2 : 1.0;
                    scratch_[u * width_ + v] = (*input_)[u * width_ + v] * k_base * su * sv;
                }
            }

            fftw_execute(plan_);
        }

    private:
        int width_;
        std::vector<double> *input_;
        std::vector<double> *output_;

        std::vector<double> scratch_;
        fftw_plan plan_ = nullptr;
        double *planned_output_ = nullptr;

        void Validate() const {
            Require(input_ != nullptr, "DctCalculator: input is null");
            Require(output_ != nullptr, "DctCalculator: output is null");
            Require(width_ > 0, "DctCalculator: width must be positive");
            Require(width_ <= SIZE_MAX, "DctCalculator: width is too large for FFTW basic interface");

            const size_t expected = width_ * width_;
            Require(input_->size() == expected, "DctCalculator: input size must be width*width");
            Require(output_->size() == expected, "DctCalculator: output size must be width*width");
        }

        void RebuildPlan() {
            if (plan_ != nullptr) {
                fftw_destroy_plan(plan_);
                plan_ = nullptr;
            }

            plan_ = fftw_plan_r2r_2d(width_, width_, scratch_.data(), output_->data(), FFTW_REDFT01,
                                     FFTW_REDFT01, FFTW_ESTIMATE);

            if (plan_ == nullptr) {
                throw std::runtime_error("DctCalculator: fftw_plan_r2r_2d failed");
            }

            planned_output_ = output_->data();
        }
    };

    DctCalculator::DctCalculator(size_t width, std::vector<double> *input, std::vector<double> *output)
        : impl_(std::make_unique<Impl>(width, input, output)) {
    }

    void DctCalculator::Inverse() const {
        impl_->Inverse();
    }

    DctCalculator::~DctCalculator() = default;
}