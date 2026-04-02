#include <jpeg_entropy.h>
#include <require.h>

int ReadSignedValue(Reader &reader, const size_t &len) {
    Require(len <= 15, "ReadTable:\ninvalid coefficient bit length");

    if (len == 0) {
        return 0;
    }

    int value = reader.ReadBit();
    for (size_t i = 1; i < len; ++i) {
        value = value * 2 + reader.ReadBit();
    }

    if ((value >> (len - 1) & 1) == 0) {
        value = value - (1 << len) + 1;
    }
    return value;
}

Table ReadTable(Reader &reader, HuffmanTree &dc, HuffmanTree &ac) {
    Table table{};

    int symbol = -1;
    while (!dc.Move(reader.ReadBit(), symbol)) {
    }

    Require(0 <= symbol && symbol <= 15, "ReadTable:\ninvalid DC symbol");
    table(0) = ReadSignedValue(reader, static_cast<size_t>(symbol));

    for (size_t i = 1; i < kTableSize;) {
        while (!ac.Move(reader.ReadBit(), symbol)) {
        }

        Require(0 <= symbol && symbol <= 255, "ReadTable:\ninvalid AC symbol");

        if (symbol == 0x00) {
            break;
        }

        if (symbol == 0xF0) {
            Require(i + 16 <= kTableSize, "ReadTable:\nZRL exceeds block");
            for (size_t j = 0; j < 16; ++j) {
                table(i++) = 0;
            }
            continue;
        }

        const size_t run = static_cast<size_t>(symbol >> 4 & 0x0F);
        const size_t len = static_cast<size_t>(symbol & 0x0F);

        Require(len != 0, "ReadTable:\ninvalid AC symbol with zero size");
        Require(i + run < kTableSize, "ReadTable:\nAC run exceeds block");

        for (size_t j = 0; j < run; ++j) {
            table(i++) = 0;
        }

        Require(i < kTableSize, "ReadTable:\ncoefficient index out of range");
        table(i++) = ReadSignedValue(reader, len);
    }

    return table;
}

void ReadMCU(Reader &reader, Context &context) {
    context.max_h = 1;
    context.max_v = 1;
    for (const uint8_t comp_id: context.scan_order) {
        const auto &[h, v, quant_table_id] = context.mcu_components.at(comp_id);
        context.max_h = std::max(context.max_h, h);
        context.max_v = std::max(context.max_v, v);
    }

    context.mcu_width = 8 * context.max_h;
    context.mcu_height = 8 * context.max_v;
    context.mcus_x = (context.width + context.mcu_width - 1) / context.mcu_width;
    context.mcus_y = (context.height + context.mcu_height - 1) / context.mcu_height;

    Require(context.mcus_x > 0, "ReadMCU:\ninvalid MCU columns");
    Require(context.mcus_y > 0, "ReadMCU:\ninvalid MCU rows");

    Require(context.mcus_x <= std::numeric_limits<size_t>::max() / context.mcus_y,
            "ReadMCU:\nMCU count overflow");

    context.total_mcus = context.mcus_x * context.mcus_y;

    Require(context.total_mcus <= 10'000'000,
            "ReadMCU:\nMCU count is too large");

    context.mcu.clear();
    context.mcu.reserve(context.total_mcus);

    std::array<int, 4> prev_dc{};

    TypeCompression compression = TypeCompression::none;
    if (context.max_h == 2 && context.max_v == 2) {
        compression = TypeCompression::all;
    } else if (context.max_h == 2) {
        compression = TypeCompression::horizontal;
    } else if (context.max_v == 2) {
        compression = TypeCompression::vertical;
    }

    size_t blocks_per_mcu = 0;
    for (const uint8_t comp_id: context.scan_order) {
        const auto &[h, v, quant_table_id] = context.mcu_components.at(comp_id);
        blocks_per_mcu += static_cast<size_t>(h) * v;
    }

    for (size_t mcu_index = 0; mcu_index < context.total_mcus; ++mcu_index) {
        MCU mcu{mcu_index, compression, {}};
        mcu.blocks.reserve(blocks_per_mcu);

        for (uint8_t comp_id: context.scan_order) {
            const auto &[h, v, quant_table_id] = context.mcu_components.at(comp_id);
            const auto &[dc_table_id, ac_table_id] = context.components.at(comp_id);

            for (size_t block = 0; block < static_cast<size_t>(h) * v; ++block) {
                Require(dc_table_id < 4, "ReadMCU:\ninvalid DC table id");
                Require(ac_table_id < 4, "ReadMCU:\ninvalid AC table id");
                Require(context.huffman_defined[0][dc_table_id], "ReadMCU:\nDC Huffman table is not defined");
                Require(context.huffman_defined[1][ac_table_id], "ReadMCU:\nAC Huffman table is not defined");
                Require(context.quant_defined[quant_table_id], "ReadMCU:\nquantization table is not defined");

                Table t = ReadTable(reader, context.huffman_trees[0][dc_table_id],
                                    context.huffman_trees[1][ac_table_id]);

                prev_dc[comp_id] += t(0);
                t(0) = prev_dc[comp_id];

                mcu.blocks.emplace_back(comp_id, t);
            }
        }

        context.mcu.push_back(std::move(mcu));
    }
}
