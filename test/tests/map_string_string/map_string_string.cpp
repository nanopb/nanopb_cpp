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

class TestMessageConverter : public AbstractMessageConverter<
        TestMessageConverter,
        LOCAL_TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
private:
    class ValuesConverter : public AbstractMapConverter<
            ValuesConverter,
            LOCAL_TestMessage::MapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    public:
        static ProtoPairType encoderInit(const EncoderContext& ctx) {
            return ProtoPairType{
                    .key = StringConverter::encoder(ctx.key),
                    .value = StringConverter::encoder(ctx.value)
            };
        }
        static ProtoPairType decoderInit(DecoderContext& ctx){
            return ProtoPairType{
                    .key = StringConverter::decoder(ctx.key),
                    .value = StringConverter::decoder(ctx.value)
            };
        }
        static bool decoderApply(const ProtoPairType& proto, DecoderContext& ctx){
            //nothing to apply
            return true;
        }
    };

public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType{
                .items = ValuesConverter::encoder(ctx.local.items)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .items = ValuesConverter::decoder(ctx.local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
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
