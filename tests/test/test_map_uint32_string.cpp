#include <stdint.h>

#include "nanopb_cpp.h"
#include "tests/helpers.h"

using MapType = std::map<uint32_t, std::string>;
using MapConverter = NanoPb::Converter::GenericMapConverter<
        MapType, MapUint32StringContainer_MapEntry, &MapUint32StringContainer_MapEntry_msg>;

int main() {
    MapType originalMap = {
            {1, "value_1" },
            {2, "value_2" }
    };
    OutputStream outputStream;

    {
        MapConverter::EncoderContext ctx(
                &originalMap,
                [](auto &k, auto &v) {
                    return MapConverter::ProtoMapEntry{
                            .key = k,
                            .value = NanoPb::Converter::StringConverter::encoder(&v)
                    };
                });

        MapUint32StringContainer msg = {
                .map = MapConverter::encoder(&ctx)
        };

        NANOPB_CPP_ASSERT(pb_encode(outputStream.getStream(), &StringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        MapConverter::DecoderContext ctx(
                &decodedMap,
                [](auto &k, auto &v ) {
                    return MapConverter::ProtoMapEntry{
                        // We have ony value as callback, so initialize decoder only on it
                        // key will be decoded by the nanopb
                        .value = NanoPb::Converter::StringConverter::decoder(&v)
                    };
                },
                [](auto &e, auto &k, auto &v){
                    return MapType::value_type(e.key, v);
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
