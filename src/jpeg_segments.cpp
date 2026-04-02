#include <jpeg_segments.h>
#include <require.h>

#include <vector>

void ReadComment(Reader &reader, Context &context) {
    const size_t length = reader.Read2Bytes();
    for (size_t i = 0; i < length - 2; i++) {
        context.comment.push_back(reader.ReadByte());
    }
}

void SkipBlock(Reader &reader) {
    const size_t length = reader.Read2Bytes();
    for (size_t i = 0; i < length - 2; i++) {
        reader.ReadByte();
    }
}

void ReadDQT(Reader &reader, Context &context) {
    size_t remaining = reader.Read2Bytes();

    remaining -= 2;

    while (0 < remaining) {
        const uint8_t byte = reader.ReadByte();

        --remaining;

        const size_t id = byte & 0x0F;
        const size_t sz = byte >> 4;

        Require(id < 4, "ReadDQT:\ninvalid quantization table id = " + std::to_string(id));
        Require(sz == 0 || sz == 1, "ReadDQT:\ninvalid quantization precision");
        Require(remaining >= kTableSize * (sz + 1), "ReadDQT:\ntruncated quantization table");

        if (sz == 0) {
            for (size_t i = 0; i < kTableSize; ++i) {
                context.quant_table[id](i) = reader.ReadByte();
            }
        } else {
            for (size_t i = 0; i < kTableSize; ++i) {
                context.quant_table[id](i) = reader.Read2Bytes();
            }
        }

        remaining -= kTableSize * (sz + 1);
        context.quant_defined[id] = true;
    }
}

void ReadSOF0(Reader &reader, Context &context) {
    Require(context.num_components == 0, "ReadSOF0:\nduplicate SOF0");

    const size_t length = reader.Read2Bytes();

    context.precision = reader.ReadByte();
    context.height = reader.Read2Bytes();
    context.width = reader.Read2Bytes();
    context.num_components = reader.ReadByte();

    Require(length == 8 + 3 * context.num_components,
            "ReadSOF0:\ninvalid segment length");
    Require(context.precision == 8, "ReadSOF0:\nonly 8-bit baseline JPEG is supported");
    Require(context.width > 0, "ReadSOF0:\nwidth must be positive");
    Require(context.height > 0, "ReadSOF0:\nheight must be positive");
    Require(context.num_components == 1 || context.num_components == 3,
            "ReadSOF0:\nunsupported number of components");

    std::array<bool, 4> used_ids{};
    for (size_t i = 0; i < context.num_components; ++i) {
        const uint8_t id = reader.ReadByte();
        const uint8_t hv = reader.ReadByte();
        const uint8_t quant_table_id = reader.ReadByte();

        const uint8_t h = hv >> 4;
        const uint8_t v = hv & 0x0F;

        Require(1 <= id && id <= 3,
                "ReadSOF0:\nunsupported component id = " + std::to_string(id));
        Require(!used_ids[id],
                "ReadSOF0:\nduplicate component id = " + std::to_string(id));
        Require(h > 0 && v > 0,
                "ReadSOF0:\ninvalid sampling factors");
        Require(quant_table_id < 4,
                "ReadSOF0:\ninvalid quantization table id = " + std::to_string(quant_table_id));

        used_ids[id] = true;
        context.mcu_components[id] = {h, v, quant_table_id};
    }
}

void ReadDHT(Reader &reader, Context &context) {
    size_t remaining = reader.Read2Bytes();

    remaining -= 2;

    while (0 < remaining) {
        const uint8_t tc_id = reader.ReadByte();

        --remaining;

        const uint8_t table_class = tc_id >> 4;
        const uint8_t id = tc_id & 0x0F;

        Require(table_class < 2, "ReadDHT:\nExecuted: table_class < 2\nActual: table_class = " +
                                 std::to_string(table_class));
        Require(id < 4, "ReadDHT:\nExecuted: id < 4\nActual: id = " + std::to_string(id));

        size_t values_count = 0;

        Require(remaining >= 17, "ReadDHT:\ntruncated header");

        std::vector<uint8_t> code_lengths(16);
        for (size_t i = 0; i < 16; ++i) {
            code_lengths[i] = reader.ReadByte();
            values_count += code_lengths[i];
        }
        remaining -= 16;

        Require(remaining >= values_count, "ReadDHT:\ntruncated values");

        std::vector<uint8_t> values(values_count);
        for (size_t i = 0; i < values_count; ++i) {
            values[i] = reader.ReadByte();
        }
        remaining -= values_count;

        context.huffman_trees[table_class][id].Build(code_lengths, values);
        context.huffman_defined[table_class][id] = true;
    }
}

void ReadSOS(Reader &reader, Context &context) {
    const size_t length = reader.Read2Bytes();
    context.num_channels = reader.ReadByte();

    Require(length == 6 + 2 * context.num_channels,
            "ReadSOS:\ninvalid segment length");
    Require(context.num_components != 0, "ReadSOS:\nSOF0 must be read before SOS");
    Require(context.num_channels == context.num_components,
            "ReadSOS:\nunsupported scan layout");

    context.scan_order.clear();

    std::array<bool, 4> seen{};
    for (size_t i = 0; i < context.num_channels; ++i) {
        const uint8_t id = reader.ReadByte();
        const uint8_t dc_ac = reader.ReadByte();

        const uint8_t dc_table_id = dc_ac >> 4;
        const uint8_t ac_table_id = dc_ac & 0x0F;

        Require(1 <= id && id <= 3,
                "ReadSOS:\nunsupported component id = " + std::to_string(id));
        Require(context.mcu_components[id].h != 0 && context.mcu_components[id].v != 0,
                "ReadSOS:\ncomponent id was not declared in SOF0: " + std::to_string(id));
        Require(!seen[id],
                "ReadSOS:\nduplicate component id in scan: " + std::to_string(id));
        Require(dc_table_id < 4,
                "ReadSOS:\ninvalid DC table id = " + std::to_string(dc_table_id));
        Require(ac_table_id < 4,
                "ReadSOS:\ninvalid AC table id = " + std::to_string(ac_table_id));

        seen[id] = true;
        context.scan_order.push_back(id);
        context.components[id] = {dc_table_id, ac_table_id};
    }

    const uint8_t ss = reader.ReadByte();
    const uint8_t se = reader.ReadByte();
    const uint8_t ah_al = reader.ReadByte();

    Require(ss == 0, "ReadSOS:\nExecuted: Ss = 0\nActual: Ss = " + std::to_string(ss));
    Require(se == 63, "ReadSOS:\nExecuted: Se = 63\nActual: Se = " + std::to_string(se));
    Require(ah_al == 0, "ReadSOS:\nExecuted: AhAl = 0\nActual: AhAl = " + std::to_string(ah_al));
}