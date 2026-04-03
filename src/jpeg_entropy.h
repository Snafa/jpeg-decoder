#pragma once

#include <reader.h>
#include <huffman.h>
#include <jpeg_types.h>

namespace jpeg_decoder {
    int ReadSignedValue(Reader &, const size_t &);

    Table ReadTable(Reader &, HuffmanTree &, HuffmanTree &);

    void ReadMCU(Reader &, Context &);
}
