#pragma once

#include "string.pb.h"

using namespace NanoPb::Converter;

struct TestMessage {
    std::string str;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;

    bool operator==(const TestMessage &rhs) const {
        return str == rhs.str;
    }

    bool operator!=(const TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public MessageConverter<
        TestMessageConverter,
        TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .str = StringConverter::encoderCallbackInit(local.str)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .str = StringConverter::decoderCallbackInit(local.str)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};
