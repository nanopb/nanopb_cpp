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

    template<class T>
    static T createTestMessages(){
        T ret;
        ret.push_back(LOCAL_InnerMessage(1, "entry_1"));
        ret.push_back(LOCAL_InnerMessage(2, "entry_2"));
        ret.push_back(LOCAL_InnerMessage(3, "entry_3"));
        return ret;
    }
};

class InnerMessageConverter : public MessageConverter<
        InnerMessageConverter,
        LOCAL_InnerMessage,
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