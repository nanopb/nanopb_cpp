#include "tests_common.h"
#include "test_map_uint32_string.pb.h"

using MapType = std::map<uint32_t, std::string>;
using MapConverter = NanoPb::Converter::GenericMapConverter<
        MapType, MapUint32StringContainer_MapEntry, &MapUint32StringContainer_MapEntry_msg>;

int main() {
    int status = 0;

    MapType originalMap = {
            {1, "value_1" },
            {2, "value_2" }
    };
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

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

        TEST(pb_encode(&outputStream, &MapUint32StringContainer_msg, &msg));
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
                    return MapConverter::LocalMapPair(e.key, v);
                });

        MapUint32StringContainer msg = {
                .map = MapConverter::decoder(&ctx)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &MapUint32StringContainer_msg, &msg));

        TEST(originalMap == decodedMap);
    }

    return status;
}
