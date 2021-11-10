#include <stdint.h>

#include "nanopb_cpp.h"
#include "tests/helpers.h"

using MapType = std::map<std::string, std::string>;
using MapConverter = NanoPb::Converter::GenericMapConverter<MapType, MapStringStringContainer_MapEntry>;

int main() {
    MapType originalMap = {
            {"key_1", "value_1" },
            {"key_2", "value_2" }
    };
    OutputStream outputStream;

    {
        MapConverter::EncoderContext ctx(
                &originalMap,
                &MapStringStringContainer_MapEntry_msg,
                [](auto &k, auto &v) {
                    return MapConverter::ProtoMapEntry {
                            .key = NanoPb::Converter::StringConverter::encoder(&k),
                            .value = NanoPb::Converter::StringConverter::encoder(&v)
                    };
                });

        MapStringStringContainer msg = {
                .map = MapConverter::encoder(&ctx)
        };

        NANOPB_CPP_ASSERT(pb_encode(outputStream.getStream(), &StringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        MapConverter::DecoderContext ctx(
                &decodedMap,
                &MapStringStringContainer_MapEntry_msg,
                [](auto &k, auto &v) {
                    return MapConverter::ProtoMapEntry {
                        // both
                        .key = NanoPb::Converter::StringConverter::decoder(&k),
                        .value = NanoPb::Converter::StringConverter::decoder(&v)
                    };
                },
                [](auto &e, auto &k, auto &v){
                    // we ignore entry because key and value were decoded by the callbacks
                    return MapType::value_type(k, v);
                });

        StringContainer msg = {
                .str = MapConverter::decoder(&ctx)
        };

        pb_istream_t stream = pb_istream_from_buffer(outputStream.getData(), outputStream.getDataSize());

        NANOPB_CPP_ASSERT(pb_decode(&stream, &StringContainer_msg, &msg));

        NANOPB_CPP_ASSERT(originalMap == decodedMap);
    }

    return 0;
}
