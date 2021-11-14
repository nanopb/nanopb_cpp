#include <map>

#include "tests_common.h"
#include "map_string_message.pb.h"
#include "common_messages.hpp"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using MapType = std::map<std::string, LOCAL_InnerMessage>;
    MapType items;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(MapType &&items) : items(std::move(items)) {}

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return items == rhs.items;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public SingleArgMessageConverter<TestMessageConverter, LOCAL_TestMessage, PROTO_TestMessage , &PROTO_TestMessage_msg> {
private:
    class ItemsConverter : public AbstractMapConverter<
            ItemsConverter,
            LOCAL_TestMessage::MapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    public:
        static ProtoPairType _encoderInit(const LocalKeyType& key, const LocalValueType& value) {
            return ProtoPairType{
                    .key = StringConverter::encoder(key),
                    .has_value = true,
                    .value = InnerMessageConverter::encoderInit(value),
            };
        }
        static ProtoPairType _decoderInit(LocalKeyType& key, LocalValueType& value){
            return ProtoPairType{
                    .key = StringConverter::decoder(key),
                    .value = InnerMessageConverter::decoderInit(value)
            };
        }
        static bool _decoderApply(const ProtoPairType& proto, LocalKeyType& key, LocalValueType& value){
            InnerMessageConverter::decoderApply(proto.value, value);
            return true;
        }
    };

public:
    static ProtoType _encoderInit(const Context& ctx) {
        return ProtoType{
                .items = ItemsConverter::encoder(ctx.items)
        };
    }

    static ProtoType _decoderInit(Context& ctx){
        return ProtoType{
                .items = ItemsConverter::decoder(ctx.items)
        };
    }

    static bool _decoderApply(const ProtoType& proto, Context& ctx){
        return true;
    }
};


int main() {
    int status = 0;

    LOCAL_TestMessage::MapType items;
    items.emplace("msg1", LOCAL_InnerMessage(1, "entry_1"));
    items.emplace("msg2", LOCAL_InnerMessage(2, "entry_2"));
    items.emplace("msg3", LOCAL_InnerMessage(3, "entry_3"));

    const LOCAL_TestMessage original(
            std::move(items)
    );

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}
