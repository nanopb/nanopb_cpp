#pragma once

#include "string.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    std::string str;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(const LOCAL_TestMessage&) = delete;
    LOCAL_TestMessage(LOCAL_TestMessage&&) = default;

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return str == rhs.str;
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
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType{
                .str = StringConverter::encoderInit(ctx.v.str)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .str = StringConverter::decoderInit(ctx.v.str)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        return true;
    }
};
