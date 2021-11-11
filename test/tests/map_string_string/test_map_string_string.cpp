#include "tests_common.h"
#include "test_map_string_string.pb.h"

using namespace NanoPb::Converter;

using MapType = std::map<std::string, std::string>;

class MapConverter : public AbstractMapConverter<
        MapConverter,
        MapType,
        MapStringStringContainer_MapEntry,
        &MapStringStringContainer_MapEntry_msg
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

    static PairType _decoderCreateMapPair(const ProtoMapEntry& entry, const KeyType& key, const ValueType& value){
        return LocalMapPair(key, value);
    }

};

int main() {
    int status = 0;

    MapType originalMap = {
            {"key_1", "value_1" },
            {"key_2", "value_2" }
    };
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        MapStringStringContainer msg = {
                .map = MapConverter::encoder(&originalMap)
        };

        TEST(pb_encode(&outputStream, &MapStringStringContainer_msg, &msg));
    }

    {
        MapType decodedMap;

        MapStringStringContainer msg = {
                .map = MapConverter::decoder(&decodedMap)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &MapStringStringContainer_msg, &msg));

        TEST(originalMap == decodedMap);
    }

    return status;
}
