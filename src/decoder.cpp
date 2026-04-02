#include <decoder.h>
#include <huffman.h>
#include <reader.h>
#include <require.h>
#include <jpeg_types.h>
#include <jpeg_segments.h>
#include <jpeg_entropy.h>
#include <jpeg_postprocess.h>

#include <glog/logging.h>

Image Decode(std::istream &input) {
    Reader reader(input);
    Context context;

    uint16_t marker = reader.ReadMarker();

    while (!reader.IsEnd() && marker != 0xFFD8) {
        marker = reader.ReadMarker();
    }

    while (true) {
        marker = reader.ReadMarker();
        if (marker == 0xFFFE) {
            ReadComment(reader, context);
        } else if (0xFFE0 <= marker && marker <= 0xFFEF) {
            SkipBlock(reader);
        } else if (marker == 0xFFDB) {
            ReadDQT(reader, context);
        } else if (marker == 0xFFC0) {
            ReadSOF0(reader, context);
        } else if (marker == 0xFFC4) {
            ReadDHT(reader, context);
        } else if (marker == 0xFFDA) {
            ReadSOS(reader, context);
            break;
        } else if (marker == 0xFFD9) {
            Error("Executed SOS marker");
        } else {
            Error("Unknown marker");
        }
    }

    ReadMCU(reader, context);

    marker = reader.ReadMarker();

    Require(marker == 0xFFD9, "Decode:\nEOI not Found!");

    Inverse(context);
    Image img = ConvertYCbCrToRgb(UpSampling(context));
    img.SetComment(context.comment);
    return img;
}
