#include <vector>

#include "tests_common.h"
#include "inner_message.hpp"
#include "array_message.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_OuterMessage {
    using ItemsContainer = std::vector<LOCAL_InnerMessage>;

    int32_t number = 0;
    ItemsContainer items;

    LOCAL_OuterMessage() = default;

    LOCAL_OuterMessage(int32_t number, ItemsContainer &&items) : number(number), items(std::move(items)) {}

    bool operator==(const LOCAL_OuterMessage &rhs) const {
        return number == rhs.number &&
               items == rhs.items;
    }

    bool operator!=(const LOCAL_OuterMessage &rhs) const {
        return !(rhs == *this);
    }

    static std::vector<LOCAL_OuterMessage> createTestMessages(){
        std::vector<LOCAL_OuterMessage> ret;
        {
            std::vector<LOCAL_InnerMessage> items;
            items.push_back(LOCAL_InnerMessage(1, "entry_1"));
            items.push_back(LOCAL_InnerMessage(2, "entry_1"));
            items.push_back(LOCAL_InnerMessage(UINT32_MAX, "entry_max"));

            ret.push_back(
                    LOCAL_OuterMessage(INT32_MIN, std::move(items))
                    );
        }
        return ret;
    }
};

class OuterMessageConverter : public AbstractMessageConverter<
        OuterMessageConverter,
        LOCAL_OuterMessage,
        PROTO_OuterMessage,
        &PROTO_OuterMessage_msg>
{
private:
    class ItemsConverter : public ArrayMessageConverter<
            ItemsConverter,
            LOCAL_OuterMessage::ItemsContainer ,
            InnerMessageConverter>
    {};

public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType{
                .number = ctx.local.number,
                .items = ItemsConverter::encoder(ctx.local.items)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .items = ItemsConverter::decoder(ctx.local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        ctx.local.number = proto.number;
        return true;
    }
};


int main() {
    int status = 0;

    const auto messages = LOCAL_OuterMessage::createTestMessages();

    for (auto& original : messages) {

        NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

        TEST(NanoPb::encode<OuterMessageConverter>(outputStream, original));

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        LOCAL_OuterMessage decoded;

        TEST(NanoPb::decode<OuterMessageConverter>(inputStream, decoded));

        TEST(original == decoded);
    }

    return status;
}
