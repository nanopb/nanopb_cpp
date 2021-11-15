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

class TestMessageConverter : public AbstractMessageConverter<
        TestMessageConverter,
        LOCAL_TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
private:
    class ItemsConverter : public AbstractMapConverter<
            ItemsConverter,
            LOCAL_TestMessage::MapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    public:
    struct DecoderContext: public AbstractMapConverter::DecoderContext {
        InnerMessageConverter::DecoderContext valueCtx;

        DecoderContext(KeyType &key, ValueType &value) :
            AbstractMapConverter::DecoderContext(key, value), valueCtx(value)
        {}
    };
    public:
        static ProtoPairType encoderInit(const EncoderContext& ctx) {
            return ProtoPairType{
                    .key = StringConverter::encoder(ctx.key),
                    .has_value = true,
                    .value = InnerMessageConverter::encoderInit(ctx.value),
            };
        }
        static ProtoPairType decoderInit(DecoderContext& ctx){
            return ProtoPairType{
                    .key = StringConverter::decoder(ctx.key),
                    .value = InnerMessageConverter::decoderInit(ctx.valueCtx)
            };
        }
        static bool decoderApply(const ProtoPairType& proto, DecoderContext& ctx){
            InnerMessageConverter::decoderApply(proto.value, ctx.valueCtx);
            return true;
        }
    };

public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType{
                .items = ItemsConverter::encoder(ctx.local.items)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .items = ItemsConverter::decoder(ctx.local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
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
