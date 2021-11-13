#include <map>

#include "tests_common.h"
#include "map_string_string.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using MapType = std::map<std::string, std::string>;
    MapType items;

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return items == rhs.items;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public AbstractMessageConverter<TestMessageConverter, LOCAL_TestMessage, PROTO_TestMessage , &PROTO_TestMessage_msg> {
private:
    friend class AbstractMessageConverter;

private:
    class ItemPairConverter : public AbstractMessageConverter<
            ItemPairConverter,
            std::pair<LocalType::MapType::key_type, LocalType::MapType::mapped_type>,
            PROTO_TestMessage_ItemsEntry ,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    private:
        friend class AbstractMessageConverter;
        static ProtoType _encoderInit(const LocalType& local) {
            return ProtoType{
                    .key = StringConverter::encoder(local.first),
                    .value = StringConverter::encoder(local.second)
            };
        }

        static ProtoType _decoderInit(LocalType& local){
            return ProtoType{
                    .key = StringConverter::decoder(local.first),
                    .value = StringConverter::decoder(local.second)
            };
        }

        static bool _decoderApply(const ProtoType& proto, LocalType& local){
            return true; //nothing to apply
        }
    };

    class ValuesConverter : public AbstractMapConverter<
            ValuesConverter,
            LOCAL_TestMessage::MapType,
            ItemPairConverter>
    {};

private:
    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .items = ValuesConverter::encoder(local.items)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .items = ValuesConverter::decoder(local.items)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


int main() {
    int status = 0;

    const LOCAL_TestMessage original = {
            .items = {
                    {"key_1", "value_1"},
                    {"key_2", "value_2"}
            }
    };

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}
