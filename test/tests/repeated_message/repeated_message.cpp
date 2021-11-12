#include <vector>

#include "tests_common.h"
#include "repeated_message.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_InnerMessage {
    uint32_t number = 0;
    std::string text;

    LOCAL_InnerMessage() = default; // Default constructor is required
    LOCAL_InnerMessage(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const LOCAL_InnerMessage &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const LOCAL_InnerMessage &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_OuterMessage {
    using ItemsContainer = std::vector<LOCAL_InnerMessage>;

    ItemsContainer items;

    bool operator==(const LOCAL_OuterMessage &rhs) const {
        return items == rhs.items;
    }

    bool operator!=(const LOCAL_OuterMessage &rhs) const {
        return !(rhs == *this);
    }
};

class InnerMessageConverter : public AbstractMessageConverter<InnerMessageConverter, LOCAL_InnerMessage, PROTO_InnerMessage, &PROTO_InnerMessage_msg> {
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .number = local.number,
                .text = StringConverter::encoder(&local.text)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
            .text = StringConverter::decoder(&local.text)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};


class OuterMessageItemsConverter : public AbstractRepeatedMessageConverter<
        OuterMessageItemsConverter,
        LOCAL_OuterMessage::ItemsContainer ,
        InnerMessageConverter>
{};

class OuterMessageConverter : public AbstractMessageConverter<OuterMessageConverter, LOCAL_OuterMessage, PROTO_OuterMessage, &PROTO_OuterMessage_msg> {
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .items = OuterMessageItemsConverter::encoder(&local.items)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .items = OuterMessageItemsConverter::decoder(&local.items)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        //nothing to apply
        return true;
    }
};


int main() {
    int status = 0;

    const LOCAL_OuterMessage original = {
            .items = {
                    LOCAL_InnerMessage(1, "entry_1"),
                    LOCAL_InnerMessage(2, "entry_1"),
                    LOCAL_InnerMessage(UINT32_MAX, "entry_max"),
            }
    };

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        auto msg = OuterMessageConverter::encoderInit(original);

        TEST(pb_encode(&outputStream, OuterMessageConverter::getMsgType(), &msg));
    }

    {
        LOCAL_OuterMessage decoded;

        auto msg = OuterMessageConverter::decoderInit(decoded);

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, OuterMessageConverter::getMsgType(), &msg));

        TEST(original == decoded);
    }

    return status;
}
