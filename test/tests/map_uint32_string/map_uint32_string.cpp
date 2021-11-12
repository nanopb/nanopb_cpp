#include <map>

#include "tests_common.h"
#include "map_uint32_string.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using MapType = std::map<uint32_t, std::string>;

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
    class ItemConverter : public AbstractMessageConverter<
            ItemConverter,
            std::pair<LocalType::MapType::key_type, LocalType::MapType::mapped_type>,
            PROTO_TestMessage_ItemsEntry ,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    private:
        friend class AbstractMessageConverter;
        static ProtoType _encoderInit(const LocalType& local) {
            return ProtoType{
                    .key = local.first,
                    .value = StringConverter::encoder(local.second)
            };
        }

        static ProtoType _decoderInit(LocalType& local){
            return ProtoType{
                    // no need to set key decoder because it is scalar type, not callback
                    .value = StringConverter::decoder(local.second)
            };
        }

        static bool _decoderApply(const ProtoType& proto, LocalType& local){
            local.first = proto.key;
            return true;
        }
    };

    class ItemsConverter : public MapConverter<
            ItemsConverter,
            LOCAL_TestMessage::MapType,
            ItemConverter>
    {};

private:
    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .items = ItemsConverter::encoder(local.items)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .items = ItemsConverter::decoder(local.items)
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
                    {1, "value_1"},
                    {2, "value_2"}
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
