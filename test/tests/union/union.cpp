#include <vector>
#include <string>
#include <memory>

#include "tests_common.h"
#include "union.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_InnerMessage {
    enum class Type {
        InnerMessageOne,
        InnerMessageTwo,
        InnerMessageThree
    };
    virtual ~LOCAL_InnerMessage(){}
    virtual Type getType() const = 0;

    template<class T>
    T* as(){ return static_cast<T*>(this); }
    template<class T>
    const T* as() const { return static_cast<const T*>(this); }

    bool operator==(const LOCAL_InnerMessage &rhs) const {
        return rhs.getType() == getType();
    }

    bool operator!=(const LOCAL_InnerMessage &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_InnerMessageOne : public LOCAL_InnerMessage {
    int number = 0;

    LOCAL_InnerMessageOne() = default;
    LOCAL_InnerMessageOne(int number) : number(number) {}

    Type getType() const override { return Type::InnerMessageOne; }

    bool operator==(const LOCAL_InnerMessageOne &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               number == rhs.number;
    }

    bool operator!=(const LOCAL_InnerMessageOne &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_InnerMessageTwo : public LOCAL_InnerMessage {
    std::string str;

    LOCAL_InnerMessageTwo() = default;
    LOCAL_InnerMessageTwo(const std::string &str) : str(str) {}

    Type getType() const override { return Type::InnerMessageTwo; }

    bool operator==(const LOCAL_InnerMessageTwo &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               str == rhs.str;
    }

    bool operator!=(const LOCAL_InnerMessageTwo &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_InnerMessageThree : public LOCAL_InnerMessage {
    using ValuesContainer = std::vector<std::string>;
    ValuesContainer values;

    LOCAL_InnerMessageThree() = default;
    LOCAL_InnerMessageThree(const ValuesContainer &values) : values(values) {}

    Type getType() const override { return Type::InnerMessageThree; }

    bool operator==(const LOCAL_InnerMessageThree &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               values == rhs.values;
    }

    bool operator!=(const LOCAL_InnerMessageThree &rhs) const {
        return !(rhs == *this);
    }
};


struct LOCAL_UnionMessage {
    std::unique_ptr<LOCAL_InnerMessage> message;

    LOCAL_UnionMessage() = default;
    LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessage> &&message) : message(std::move(message)) {}

    bool operator==(const LOCAL_UnionMessage &rhs) const {
        if (!rhs.message && !message){
            return true;
        }
        if (!rhs.message || !message){
            return false;
        }
        if (rhs.message->getType() != message->getType()){
            return false;
        }
        switch (this->message->getType()) {
            case LOCAL_InnerMessage::Type::InnerMessageOne:
                return *rhs.message->as<LOCAL_InnerMessageOne>() == *message->as<LOCAL_InnerMessageOne>();
            case LOCAL_InnerMessage::Type::InnerMessageTwo:
                return *rhs.message->as<LOCAL_InnerMessageTwo>() == *message->as<LOCAL_InnerMessageTwo>();
            case LOCAL_InnerMessage::Type::InnerMessageThree:
                return *rhs.message->as<LOCAL_InnerMessageThree>() == *message->as<LOCAL_InnerMessageThree>();
        }
        NANOPB_CPP_ASSERT(0&&"Invalid type");
        return false;
    }

    bool operator!=(const LOCAL_UnionMessage &rhs) const {
        return !(rhs == *this);
    }
};

/******************************************************************************************/

class InnerMessageOneConverter : public AbstractMessageConverter<
        InnerMessageOneConverter,
        LOCAL_InnerMessageOne,
        PROTO_InnerMessageOne ,
        &PROTO_InnerMessageOne_msg>
{
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType {
            .number = local.number
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};

class InnerMessageTwoConverter : public AbstractMessageConverter<
        InnerMessageTwoConverter,
        LOCAL_InnerMessageTwo,
        PROTO_InnerMessageTwo ,
        &PROTO_InnerMessageTwo_msg>
{
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType {
                .str = StringConverter::encoder(local.str)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .str = StringConverter::decoder(local.str)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};

class InnerMessageThreeConverter : public AbstractMessageConverter<
        InnerMessageThreeConverter,
        LOCAL_InnerMessageThree,
        PROTO_InnerMessageThree ,
        &PROTO_InnerMessageThree_msg>
{
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType {
                .values = ArrayStringConverter<LOCAL_InnerMessageThree::ValuesContainer>::encoder(local.values)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .values = ArrayStringConverter<LOCAL_InnerMessageThree::ValuesContainer>::decoder(local.values)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


class UnionMessageConverter : public AbstractMessageConverter<
        UnionMessageConverter,
        LOCAL_UnionMessage,
        PROTO_UnionMessage ,
        &PROTO_UnionMessage_msg>
{
private:
    friend class AbstractMessageConverter;
private:
    static ProtoType _encoderInit(const LocalType& local) {
        ProtoType ret {
                .which_msg = 0
        };
        if (!local.message) {
            return ret;
        }
        switch (local.message->getType()) {
            case LOCAL_InnerMessage::Type::InnerMessageOne:
                ret.which_msg = PROTO_UnionMessage_msg1_tag;
                ret.msg.msg1 = InnerMessageOneConverter::encoderInit(*local.message->as<LOCAL_InnerMessageOne>());
                break;
            case LOCAL_InnerMessage::Type::InnerMessageTwo:
                ret.which_msg = PROTO_UnionMessage_msg2_tag;
                ret.msg.msg2 = InnerMessageTwoConverter::encoderInit(*local.message->as<LOCAL_InnerMessageTwo>());
                break;
            case LOCAL_InnerMessage::Type::InnerMessageThree:
                ret.which_msg = PROTO_UnionMessage_msg3_tag;
                ret.msg.msg3 = InnerMessageThreeConverter::encoderInit(*local.message->as<LOCAL_InnerMessageThree>());
                break;
        }
        return ret;
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
            //FIXME: Not implemented
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


int testMessage(const LOCAL_UnionMessage& original){
    int status = 0;


    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<UnionMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionMessage decoded;

    TEST(NanoPb::decode<UnionMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}


int main() {
    int status = 0;

    COMMENT("LOCAL_InnerMessageOne");

    status |= testMessage(LOCAL_UnionMessage(
            std::unique_ptr<LOCAL_InnerMessageOne>(new LOCAL_InnerMessageOne(111))
                    ));

    COMMENT("LOCAL_InnerMessageTwo");
    status |= testMessage(LOCAL_UnionMessage(
            std::unique_ptr<LOCAL_InnerMessageTwo>(new LOCAL_InnerMessageTwo("Message number two"))
    ));

    COMMENT("LOCAL_InnerMessageThree");
    status |= testMessage(LOCAL_UnionMessage(
            std::unique_ptr<LOCAL_InnerMessageThree>(new LOCAL_InnerMessageThree({"string_1", "string_2"}))
    ));

    return status;
}
