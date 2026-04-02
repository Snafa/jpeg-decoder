#include <huffman.h>
#include <stdexcept>

class HuffmanTree::Impl {
public:
    Impl() = default;

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    Impl(Impl &&) = default;
    Impl &operator=(Impl &&) = default;

    ~Impl() = default;

    Impl(const std::vector<uint8_t> &code_lengths, const std::vector<uint8_t> &values) {
        if (code_lengths.size() > 16) {
            throw std::invalid_argument("too many code lengths");
        }

        size_t total = 0, max_count = 2;
        for (uint8_t x : code_lengths) {
            total += x;
            if (max_count < x) {
                throw std::invalid_argument("too many leafs");
            }
            max_count = (max_count - x) << 1;
        }

        if (total != values.size()) {
            throw std::invalid_argument("values size mismatch");
        }

        values_ = values;
        code_lengths_ = code_lengths;
    }

    void Reset() {
        current_num_ = 0;
        current_sum_ = 0;
        current_level_ = 0;
        current_left_num_ = 0;
    }

    bool Move(bool bit, int &value) {
        if (code_lengths_.size() <= current_level_) {
            throw std::invalid_argument("HuffmanTree::Move()");
        }

        current_num_ = current_num_ << 1 | bit;
        current_left_num_ = current_left_num_ << 1;

        if (current_num_ < current_left_num_ + code_lengths_[current_level_]) {
            value = values_[current_num_ - current_left_num_ + current_sum_];
            Reset();
            return true;
        }

        current_left_num_ += code_lengths_[current_level_];
        current_sum_ += code_lengths_[current_level_];
        current_level_++;

        return false;
    }

private:
    std::vector<uint8_t> code_lengths_, values_;
    size_t current_left_num_ = 0, current_num_ = 0, current_sum_ = 0, current_level_ = 0;
};

HuffmanTree::HuffmanTree() : impl_(std::make_unique<Impl>()) {
}

void HuffmanTree::Build(const std::vector<uint8_t> &code_lengths,
                        const std::vector<uint8_t> &values) {
    impl_ = std::make_unique<Impl>(code_lengths, values);
}

bool HuffmanTree::Move(bool bit, int &value) {
    return impl_->Move(bit, value);
}

HuffmanTree::HuffmanTree(HuffmanTree &&) = default;

HuffmanTree &HuffmanTree::operator=(HuffmanTree &&) = default;

HuffmanTree::~HuffmanTree() = default;
