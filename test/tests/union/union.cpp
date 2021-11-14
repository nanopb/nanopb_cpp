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


struct UnionMessageConverterContext {
    struct Encoder {
        const LOCAL_UnionMessage& local;

        Encoder(const LOCAL_UnionMessage &local) : local(local) {}
    };
    struct Decoder {
        LOCAL_UnionMessage& local;
        std::unique_ptr<LOCAL_InnerMessageOne> msg1;
        std::unique_ptr<LOCAL_InnerMessageTwo> msg2;
        std::unique_ptr<LOCAL_InnerMessageThree> msg3;

        Decoder(const Decoder&) = delete;
        Decoder(LOCAL_UnionMessage &local) :
            local(local),
            msg1(new LOCAL_InnerMessageOne()),
            msg2(new LOCAL_InnerMessageTwo()),
            msg3(new LOCAL_InnerMessageThree())
        {
        }
    };

    std::unique_ptr<Encoder> encoder;
    std::unique_ptr<Decoder> decoder;

private:
    UnionMessageConverterContext(std::unique_ptr<Encoder>&& encoder) : encoder(std::move(encoder)) {}
    UnionMessageConverterContext(std::unique_ptr<Decoder>&& decoder) : decoder(std::move(decoder)) {}
public:

    static UnionMessageConverterContext createEncoder(const LOCAL_UnionMessage& local){
        return UnionMessageConverterContext(std::unique_ptr<Encoder>(new Encoder(local)));
    }

    static UnionMessageConverterContext createDecoder(LOCAL_UnionMessage& local){
        return UnionMessageConverterContext(std::unique_ptr<Decoder>(new Decoder(local)));
    }
};

class UnionMessageConverter : public AbstractMessageContextConverter<
        UnionMessageConverter,
        UnionMessageConverterContext,
        PROTO_UnionMessage ,
        &PROTO_UnionMessage_msg>
{
private:
    friend class AbstractMessageContextConverter;
private:
    static ProtoType _encoderInit(const Context& ctx) {
        auto& local = ctx.encoder->local;
        ProtoType ret {};
        NANOPB_CPP_ASSERT(local.message);
        if (!local.message) {
            return ret;
        }
        switch (local.message->getType()) {
            case LOCAL_InnerMessage::Type::InnerMessageOne:
                ret.has_msg1 = true;
                ret.msg1 = InnerMessageOneConverter::encoderInit(*local.message->as<LOCAL_InnerMessageOne>());
                break;
            case LOCAL_InnerMessage::Type::InnerMessageTwo:
                ret.has_msg2 = true;
                ret.msg2 = InnerMessageTwoConverter::encoderInit(*local.message->as<LOCAL_InnerMessageTwo>());
                break;
            case LOCAL_InnerMessage::Type::InnerMessageThree:
                ret.has_msg3 = true;
                ret.msg3 = InnerMessageThreeConverter::encoderInit(*local.message->as<LOCAL_InnerMessageThree>());
                break;
        }
        return ret;
    }

    static ProtoType _decoderInit(Context& ctx){
        return ProtoType{
            .msg1 = InnerMessageOneConverter::decoderInit(*ctx.decoder->msg1),
            .msg2 = InnerMessageTwoConverter::decoderInit(*ctx.decoder->msg2),
            .msg3 = InnerMessageThreeConverter::decoderInit(*ctx.decoder->msg3)
        };
    }

    static bool _decoderApply(const ProtoType& proto, Context& ctx){
        if (proto.has_msg1){
            InnerMessageOneConverter::decoderApply(proto.msg1,*ctx.decoder->msg1),
            ctx.decoder->local.message = std::move(ctx.decoder->msg1);
            return true;
        } else if (proto.has_msg2){
            InnerMessageTwoConverter::decoderApply(proto.msg2, *ctx.decoder->msg2),
            ctx.decoder->local.message = std::move(ctx.decoder->msg2);
            return true;
        } else if (proto.has_msg3){
            InnerMessageThreeConverter::decoderApply(proto.msg3, *ctx.decoder->msg3),
            ctx.decoder->local.message = std::move(ctx.decoder->msg3);
            return true;
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid msg");
            return false;
        }
    }
};


int testMessage(const LOCAL_UnionMessage& original){
    int status = 0;


    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    auto encoderContext = UnionMessageConverterContext::createEncoder(original);

    TEST(NanoPb::encode<UnionMessageConverter>(outputStream, encoderContext));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionMessage decoded;

    auto decoderContext = UnionMessageConverterContext::createDecoder(decoded);

    TEST(NanoPb::decode<UnionMessageConverter>(inputStream, decoderContext));

    TEST(original == decoded);

    return status;
}


int main() {
    int status = 0;

    COMMENT("LOCAL_InnerMessageOne");

    auto msg1 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageOne>(
                    new LOCAL_InnerMessageOne(111)
            ));
    status |= testMessage(msg1);

    COMMENT("LOCAL_InnerMessageTwo");
    auto msg2 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageTwo>(
            new LOCAL_InnerMessageTwo("Message number two")
    ));
    status |= testMessage(msg2);

    COMMENT("LOCAL_InnerMessageThree");
    auto msg3 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageThree>(
            new LOCAL_InnerMessageThree({"string_1", "string_2"})
    ));
    status |= testMessage(msg3);

    return status;
}
