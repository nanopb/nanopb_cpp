#include <map>

#include "tests_common.h"
#include "map_string_string.pb.h"

using namespace NanoPb::Converter;

using MapType = std::map<std::string, std::string>;

class MapConverter : public AbstractMapConverter<
        MapConverter,
        MapType,
        PROTO_MapStringStringContainer_MapEntry,
        &PROTO_MapStringStringContainer_MapEntry_msg
>
{
private:
    friend class AbstractMapConverter;

    static ProtoMapEntry _encoderInitializer(const KeyType& key, const ValueType& value){
        return ProtoMapEntry{
                .key = StringConverter::encoder(&key),
                .value = StringConverter::encoder(&value)
        };
    }

    static ProtoMapEntry _decoderInitializer(KeyType& key, ValueType& value){
        return ProtoMapEntry{
                .key = StringConverter::decoder(&key),
                .value = StringConverter::decoder(&value)
        };
    }

    static PairType _decoderCreateMapPair(const ProtoMapEntry& protoMapEntry, const KeyType& key, const ValueType& value){
        return LocalMapPair(key, value);
    }

};

int main() {
    int status = 0;

    const MapType originalMap = {
            {"key_1", "value_1" },
            {"key_2", "value_2" }
    };
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        PROTO_MapStringStringContainer msg = {
                .map = MapConverter::encoder(&originalMap)
        };

        TEST(pb_encode(&outputStream, &PROTO_MapStringStringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        PROTO_MapStringStringContainer msg = {
                .map = MapConverter::decoder(&decodedMap)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &PROTO_MapStringStringContainer_msg, &msg));

        TEST(originalMap == decodedMap);
    }

    return status;
}
