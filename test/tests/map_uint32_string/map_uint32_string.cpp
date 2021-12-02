#include <map>

#include "tests_common.h"
#include "map_uint32_string.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using SimpleMapType = std::map<uint32_t, std::string>;

    SimpleMapType items;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(const LOCAL_TestMessage&) = delete;
    LOCAL_TestMessage(LOCAL_TestMessage&&) = default;

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
    class SimpleItemsConverter : public MapConverter<
            SimpleItemsConverter,
            LOCAL_TestMessage::SimpleMapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>
    {
    public:
        static ProtoPairType itemEncoderInit(const ItemEncoderContext& localPair) {
            return ProtoPairType{
                    .key = localPair.key,
                    .value = StringConverter::encoderInit(localPair.value)
            };
        }
        static ProtoPairType itemDecoderInit(ItemDecoderContext& localPair){
            return ProtoPairType{
                    // no need to set key decoder because it is scalar type, not callback
                    .value = StringConverter::decoderInit(localPair.value)
            };
        }
        static bool itemDecoderApply(const ProtoPairType& proto, ItemDecoderContext& localPair){
            localPair.key = proto.key;
            return true;
        }
    };

public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .items = SimpleItemsConverter::encoderInit(local.items)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .items = SimpleItemsConverter::decoderInit(local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
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
