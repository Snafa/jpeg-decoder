#include <jpeg_postprocess.h>
#include <fft.h>
#include <require.h>

namespace jpeg_decoder {
    void Inverse(Context &context) {
        std::vector<double> in(kTableSize), out(kTableSize);
        const DctCalculator calc(8, &in, &out);
        for (auto &mcu: context.mcu) {
            for (auto &[comp_id, table]: mcu.blocks) {
                const auto &[h, v, quant_table_id] = context.mcu_components.at(comp_id);
                const Table &quant_table = context.quant_table[quant_table_id];

                Require(context.quant_defined[quant_table_id],
                        "Inverse:\nquantization table is not defined");

                for (size_t i = 0; i < 8; ++i) {
                    for (size_t j = 0; j < 8; ++j) {
                        in[i * 8 + j] = table(i, j) * quant_table(i, j);
                    }
                }

                calc.Inverse();

                for (size_t i = 0; i < 8; ++i) {
                    for (size_t j = 0; j < 8; ++j) {
                        table(i, j) = Clamp(out[i * 8 + j] + 128);
                    }
                }
            }
        }
    }

    std::vector<std::vector<YCbCr> > UpSampling(const Context &context) {
        std::vector img(context.height, std::vector(context.width, YCbCr{0, 128, 128}));
        for (size_t mcu_index = 0; mcu_index < context.mcu.size(); ++mcu_index) {
            const auto &mcu = context.mcu[mcu_index];

            const size_t mcu_x = mcu_index % context.mcus_x;
            const size_t mcu_y = mcu_index / context.mcus_x;

            std::array<size_t, 4> local_block_index{};

            for (const auto &[comp_id, tab]: mcu.blocks) {
                const auto &comp = context.mcu_components.at(comp_id);

                const size_t h = comp.h;
                const size_t v = comp.v;

                const size_t block_index = local_block_index[comp_id]++;
                const size_t block_x = block_index % h;
                const size_t block_y = block_index / h;

                const size_t scale_x = context.max_h / h;
                const size_t scale_y = context.max_v / v;

                for (size_t yy = 0; yy < 8; ++yy) {
                    for (size_t xx = 0; xx < 8; ++xx) {
                        const int value = tab(yy, xx);

                        const size_t base_x =
                                mcu_x * context.mcu_width + block_x * 8 * scale_x + xx * scale_x;
                        const size_t base_y =
                                mcu_y * context.mcu_height + block_y * 8 * scale_y + yy * scale_y;

                        for (size_t dy = 0; dy < scale_y; ++dy) {
                            for (size_t dx = 0; dx < scale_x; ++dx) {
                                const size_t x = base_x + dx;
                                const size_t y = base_y + dy;

                                if (y >= context.height || x >= context.width) {
                                    continue;
                                }

                                if (comp_id == 1) {
                                    img[y][x].y = value;
                                } else if (comp_id == 2) {
                                    img[y][x].cb = value;
                                } else if (comp_id == 3) {
                                    img[y][x].cr = value;
                                } else {
                                    Error("UpSampling:\nunsupported component id");
                                }
                            }
                        }
                    }
                }
            }
        }

        return img;
    }

    Image ConvertYCbCrToRgb(const std::vector<std::vector<YCbCr> > &data) {
        Image img(data[0].size(), data.size());
        for (size_t h = 0; h < data.size(); ++h) {
            for (size_t v = 0; v < data[h].size(); ++v) {
                img.Pixel(h, v) = static_cast<RGB>(data[h][v]);
            }
        }
        return img;
    }
}
