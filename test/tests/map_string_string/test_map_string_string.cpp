#include "tests_common.h"
#include "test_map_string_string.pb.h"

using MapType = std::map<std::string, std::string>;
using MapConverter = NanoPb::Converter::GenericMapConverter<
        MapType, MapStringStringContainer_MapEntry, &MapStringStringContainer_MapEntry_msg>;

int main() {
    MapType originalMap = {
            {"key_1", "value_1" },
            {"key_2", "value_2" }
    };
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        MapConverter::EncoderContext ctx(
                &originalMap,
                [](auto &k, auto &v) {
                    return MapConverter::ProtoMapEntry {
                            .key = NanoPb::Converter::StringConverter::encoder(&k),
                            .value = NanoPb::Converter::StringConverter::encoder(&v)
                    };
                });

        MapStringStringContainer msg = {
                .map = MapConverter::encoder(&ctx)
        };

        NANOPB_CPP_ASSERT(pb_encode(&outputStream, &MapStringStringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        MapConverter::DecoderContext ctx(
                &decodedMap,
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

        MapStringStringContainer msg = {
                .map = MapConverter::decoder(&ctx)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        NANOPB_CPP_ASSERT(pb_decode(&inputStream, &MapStringStringContainer_msg, &msg));

        NANOPB_CPP_ASSERT(originalMap == decodedMap);
    }

    return 0;
}
