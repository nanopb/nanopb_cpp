#pragma once

#include <vector>
#include "tests_common.h"
#include "inner_message.pb.h"

using namespace NanoPb::Converter;

struct InnerMessage {
    uint32_t number = 0;
    std::string text;

    InnerMessage() = default; // Default constructor is required

    // Remove copy constructor and add move constructor
    // to ensure that all can work without copy constructor
    InnerMessage(const InnerMessage&) = delete;
    InnerMessage(InnerMessage&& other) = default;

    InnerMessage(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const InnerMessage &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const InnerMessage &rhs) const {
        return !(rhs == *this);
    }

    template<class T>
    static T createTestMessages(){
        T ret;
        ret.push_back(InnerMessage(1, "entry_1"));
        ret.push_back(InnerMessage(2, "entry_2"));
        ret.push_back(InnerMessage(3, "entry_3"));
        return ret;
    }
};

class InnerMessageConverter : public MessageConverter<
        InnerMessageConverter,
        InnerMessage,
        PROTO_InnerMessage,
        &PROTO_InnerMessage_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .number = local.number,
                .text = StringConverter::encoderCallbackInit(local.text)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .text = StringConverter::decoderCallbackInit(local.text)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};