#pragma once

#include <reader.h>
#include <jpeg_types.h>

namespace jpeg_decoder {
    void ReadComment(Reader &, Context &);

    void SkipBlock(Reader &);

    void ReadDQT(Reader &, Context &);

    void ReadSOF0(Reader &, Context &);

    void ReadDHT(Reader &, Context &);

    void ReadSOS(Reader &, Context &);
}
