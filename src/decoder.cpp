#include <fstream>
#include <jpeg_decoder/decoder.h>
#include <huffman.h>
#include <reader.h>
#include <require.h>
#include <jpeg_types.h>
#include <jpeg_segments.h>
#include <jpeg_entropy.h>
#include <jpeg_postprocess.h>

#include <glog/logging.h>

namespace jpeg_decoder {
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
        img.Comment() = context.comment;
        return img;
    }

    Image DecodeFile(const std::string& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("Failed to open file: " + path);
        }
        return Decode(input);
    }
}
