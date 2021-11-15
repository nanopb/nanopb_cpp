#pragma once

#include "union_message.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_InnerMessage {
    enum class Type {
        UnionInnerOne,
        UnionInnerTwo,
        UnionInnerThree
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

struct LOCAL_UnionInnerOne : public LOCAL_InnerMessage {
    int number = 0;

    LOCAL_UnionInnerOne() = default;
    LOCAL_UnionInnerOne(int number) : number(number) {}

    Type getType() const override { return Type::UnionInnerOne; }

    bool operator==(const LOCAL_UnionInnerOne &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               number == rhs.number;
    }

    bool operator!=(const LOCAL_UnionInnerOne &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_UnionInnerTwo : public LOCAL_InnerMessage {
    std::string str;

    LOCAL_UnionInnerTwo() = default;
    LOCAL_UnionInnerTwo(const std::string &str) : str(str) {}

    Type getType() const override { return Type::UnionInnerTwo; }

    bool operator==(const LOCAL_UnionInnerTwo &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               str == rhs.str;
    }

    bool operator!=(const LOCAL_UnionInnerTwo &rhs) const {
        return !(rhs == *this);
    }
};

struct LOCAL_UnionInnerThree : public LOCAL_InnerMessage {
    using ValuesContainer = std::vector<uint32_t>;
    ValuesContainer values;

    LOCAL_UnionInnerThree() = default;
    LOCAL_UnionInnerThree(const ValuesContainer &values) : values(values) {}

    Type getType() const override { return Type::UnionInnerThree; }

    bool operator==(const LOCAL_UnionInnerThree &rhs) const {
        return static_cast<const LOCAL_InnerMessage &>(*this) == static_cast<const LOCAL_InnerMessage &>(rhs) &&
               values == rhs.values;
    }

    bool operator!=(const LOCAL_UnionInnerThree &rhs) const {
        return !(rhs == *this);
    }
};


struct LOCAL_UnionContainer {
    std::unique_ptr<LOCAL_InnerMessage> message;

    LOCAL_UnionContainer() = default;
    LOCAL_UnionContainer(std::unique_ptr<LOCAL_InnerMessage> &&message) : message(std::move(message)) {}

    bool operator==(const LOCAL_UnionContainer &rhs) const {
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
            case LOCAL_InnerMessage::Type::UnionInnerOne:
                return *rhs.message->as<LOCAL_UnionInnerOne>() == *message->as<LOCAL_UnionInnerOne>();
            case LOCAL_InnerMessage::Type::UnionInnerTwo:
                return *rhs.message->as<LOCAL_UnionInnerTwo>() == *message->as<LOCAL_UnionInnerTwo>();
            case LOCAL_InnerMessage::Type::UnionInnerThree:
                return *rhs.message->as<LOCAL_UnionInnerThree>() == *message->as<LOCAL_UnionInnerThree>();
        }
        NANOPB_CPP_ASSERT(0&&"Invalid type");
        return false;
    }

    bool operator!=(const LOCAL_UnionContainer &rhs) const {
        return !(rhs == *this);
    }
};

/******************************************************************************************/

class UnionInnerOneConverter : public AbstractMessageConverter<
        UnionInnerOneConverter,
        LOCAL_UnionInnerOne,
        PROTO_UnionInnerOne,
        &PROTO_UnionInnerOne_msg>
{
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType {
                .number = ctx.local.number
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        ctx.local.number = proto.number;
        return true;
    }
};

class UnionInnerTwoConverter : public AbstractMessageConverter<
        UnionInnerTwoConverter,
        LOCAL_UnionInnerTwo,
        PROTO_UnionInnerTwo,
        &PROTO_UnionInnerTwo_msg>
{
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType {
                .str = StringConverter::encoder(ctx.local.str)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .str = StringConverter::decoder(ctx.local.str)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        return true;
    }
};

class UnionInnerThreeConverter : public AbstractMessageConverter<
        UnionInnerThreeConverter,
        LOCAL_UnionInnerThree,
        PROTO_UnionInnerThree,
        &PROTO_UnionInnerThree_msg>
{
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {
        return ProtoType {
                .values = ArrayUnsignedConverter<LOCAL_UnionInnerThree::ValuesContainer>::encoder(ctx.local.values)
        };
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .values = ArrayUnsignedConverter<LOCAL_UnionInnerThree::ValuesContainer>::decoder(ctx.local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        return true;
    }
};

class UnionContainerConverter : public AbstractMessageConverter<
        UnionContainerConverter,
        LOCAL_UnionContainer,
        PROTO_UnionContainer,
        &PROTO_UnionContainer_msg>
{
public:
    struct DecoderContext : public AbstractMessageConverter::DecoderContext {
        std::unique_ptr<LOCAL_UnionInnerOne> msg1;
        std::unique_ptr<LOCAL_UnionInnerTwo> msg2;
        std::unique_ptr<LOCAL_UnionInnerThree> msg3;

        struct MessageContexts {
            UnionInnerOneConverter::DecoderContext msg1;
            UnionInnerTwoConverter::DecoderContext msg2;
            UnionInnerThreeConverter::DecoderContext msg3;

            MessageContexts(LOCAL_UnionInnerOne& msg1, LOCAL_UnionInnerTwo& msg2, LOCAL_UnionInnerThree& msg3) :
                    msg1(msg1), msg2(msg2), msg3(msg3)
            {}
        } messageContexts;

        DecoderContext(const DecoderContext&) = delete;
        DecoderContext(LocalType& local) :
                AbstractMessageConverter::DecoderContext(local),
                msg1(new LOCAL_UnionInnerOne()),
                msg2(new LOCAL_UnionInnerTwo()),
                msg3(new LOCAL_UnionInnerThree()),
                messageContexts(*msg1, *msg2, *msg3)
        {
        }
    };
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {

        ProtoType ret {};
        NANOPB_CPP_ASSERT(ctx.local.message);
        if (!ctx.local.message) {
            return ret;
        }
        switch (ctx.local.message->getType()) {
            case LOCAL_InnerMessage::Type::UnionInnerOne:
                ret.has_msg1 = true;
                ret.msg1 = UnionInnerOneConverter::encoderInit(*ctx.local.message->as<LOCAL_UnionInnerOne>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerTwo:
                ret.has_msg2 = true;
                ret.msg2 = UnionInnerTwoConverter::encoderInit(*ctx.local.message->as<LOCAL_UnionInnerTwo>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerThree:
                ret.has_msg3 = true;
                ret.msg3 = UnionInnerThreeConverter::encoderInit(*ctx.local.message->as<LOCAL_UnionInnerThree>());
                break;
        }
        return ret;
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        return ProtoType{
                .msg1 = UnionInnerOneConverter::decoderInit(ctx.messageContexts.msg1),
                .msg2 = UnionInnerTwoConverter::decoderInit(ctx.messageContexts.msg2),
                .msg3 = UnionInnerThreeConverter::decoderInit(ctx.messageContexts.msg3)
        };
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        if (proto.has_msg1){
            UnionInnerOneConverter::decoderApply(proto.msg1,ctx.messageContexts.msg1);
            ctx.local.message = std::move(ctx.msg1);
            return true;
        } else if (proto.has_msg2){
            UnionInnerTwoConverter::decoderApply(proto.msg2, ctx.messageContexts.msg2);
            ctx.local.message = std::move(ctx.msg2);
            return true;
        } else if (proto.has_msg3){
            UnionInnerThreeConverter::decoderApply(proto.msg3, ctx.messageContexts.msg3);
            ctx.local.message = std::move(ctx.msg3);
            return true;
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid msg");
            return false;
        }
    }
};