#include "tests_common.h"
#include "common_messages.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_InnerMessage {
    uint32_t number = 0;
    std::string text;

    LOCAL_InnerMessage() = default; // Default constructor is required

    // Remove copy constructor and add move constructor
    // to ensure that all can work without copy constructor
    LOCAL_InnerMessage(const LOCAL_InnerMessage&) = delete;
    LOCAL_InnerMessage(LOCAL_InnerMessage&& other) : number(other.number), text(std::move(other.text)) {};

    LOCAL_InnerMessage(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const LOCAL_InnerMessage &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const LOCAL_InnerMessage &rhs) const {
        return !(rhs == *this);
    }
};

class InnerMessageConverter : public AbstractMessageConverter<
        InnerMessageConverter,
        LOCAL_InnerMessage,
        PROTO_InnerMessage,
        &PROTO_InnerMessage_msg>
{
public:
    static ProtoType encoderInit(const EncoderContext & ctx) {
        return ProtoType{
                .number = ctx.local.number,
                .text = StringConverter::encoder(ctx.local.text)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .text = StringConverter::decoder(ctx.local.text)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        ctx.local.number = proto.number;
        return true;
    }
};