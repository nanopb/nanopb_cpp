#include <map>

#include "tests_common.h"
#include "map_uint32_string.pb.h"

using namespace NanoPb::Converter;

using MapType = std::map<uint32_t, std::string>;

class MapConverter : public AbstractMapConverter<
        MapConverter,
        MapType,
        PROTO_MapUint32StringContainer_MapEntry,
        &PROTO_MapUint32StringContainer_MapEntry_msg
        >
{
private:
    friend class AbstractMapConverter;

    static ProtoMapEntry _encoderInitializer(const KeyType& key, const ValueType& value){
        return ProtoMapEntry{
                .key = key,
                .value = StringConverter::encoder(&value)
        };
    }

    static ProtoMapEntry _decoderInitializer(KeyType& key, ValueType& value){
        return ProtoMapEntry{
                // key is scalar type and will be decoded by nanopb, so we don't need to initialize it
                // value is callback type
                .value = StringConverter::decoder(&value)
        };
    }

    static PairType _decoderCreateMapPair(const ProtoMapEntry& protoMapEntry, const KeyType& key, const ValueType& value){
        // We take scalar type key directly from  entry
        // and take value which had callback decoder from callback result
        return LocalMapPair(protoMapEntry.key, value);
    }

};

int main() {
    int status = 0;

    const MapType originalMap = {
            {1, "value_1" },
            {2, "value_2" }
    };
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        PROTO_MapUint32StringContainer msg = {
                .map = MapConverter::encoder(&originalMap)
        };

        TEST(pb_encode(&outputStream, &PROTO_MapUint32StringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        PROTO_MapUint32StringContainer msg = {
                .map = MapConverter::decoder(&decodedMap)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &PROTO_MapUint32StringContainer_msg, &msg));

        TEST(originalMap == decodedMap);
    }

    return status;
}
