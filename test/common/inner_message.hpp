#pragma once

#include <vector>
#include "tests_common.h"
#include "inner_message.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_InnerMessage {
    uint32_t number = 0;
    std::string text;

    LOCAL_InnerMessage() = default; // Default constructor is required

    // Remove copy constructor and add move constructor
    // to ensure that all can work without copy constructor
    LOCAL_InnerMessage(const LOCAL_InnerMessage&) = delete;
    LOCAL_InnerMessage(LOCAL_InnerMessage&& other) = default;

    LOCAL_InnerMessage(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const LOCAL_InnerMessage &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const LOCAL_InnerMessage &rhs) const {
        return !(rhs == *this);
    }

    static std::vector<LOCAL_InnerMessage> createTestMessages(){
        std::vector<LOCAL_InnerMessage> ret;
        ret.push_back(LOCAL_InnerMessage(1, "entry_1"));
        ret.push_back(LOCAL_InnerMessage(2, "entry_2"));
        ret.push_back(LOCAL_InnerMessage(3, "entry_3"));
        return ret;
    }
};

class InnerMessageConverter : public AbstractMessageConverter<
        InnerMessageConverter,
        LOCAL_InnerMessage,
        PROTO_InnerMessage,
        &PROTO_InnerMessage_msg>
{
public:
    static ProtoType encoderInit(EncoderContext& ctx) {
        return ProtoType{
                .number = ctx.number,
                .text = StringConverter::encoderInit(ctx.text)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .text = StringConverter::decoderInit(ctx.text)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        ctx.number = proto.number;
        return true;
    }
};