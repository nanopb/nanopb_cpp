#pragma once

#include "container_no_union.pb.h"
#include "container.hpp"

using namespace NanoPb::Converter;

class UnionContainerNoUnionConverter : public AbstractMessageConverter<
        UnionContainerNoUnionConverter,
        LOCAL_UnionContainer,
        PROTO_UnionContainerNoUnion,
        &PROTO_UnionContainerNoUnion_msg>
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
        DecoderContext(LocalType& v) :
                AbstractMessageConverter::DecoderContext(v),
                msg1(new LOCAL_UnionInnerOne()),
                msg2(new LOCAL_UnionInnerTwo()),
                msg3(new LOCAL_UnionInnerThree()),
                messageContexts(*msg1, *msg2, *msg3)
        {
        }
    };
public:
    static ProtoType encoderInit(const EncoderContext& ctx) {

        ProtoType ret {
            .prefix = ctx.v.prefix,
            .suffix = ctx.v.suffix
        };
        NANOPB_CPP_ASSERT(ctx.v.message);
        if (!ctx.v.message) {
            return ret;
        }
        switch (ctx.v.message->getType()) {
            case LOCAL_InnerMessage::Type::UnionInnerOne:
                ret.has_msg1 = true;
                ret.msg1 = UnionInnerOneConverter::encoderInit(*ctx.v.message->as<LOCAL_UnionInnerOne>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerTwo:
                ret.has_msg2 = true;
                ret.msg2 = UnionInnerTwoConverter::encoderInit(*ctx.v.message->as<LOCAL_UnionInnerTwo>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerThree:
                ret.has_msg3 = true;
                ret.msg3 = UnionInnerThreeConverter::encoderInit(*ctx.v.message->as<LOCAL_UnionInnerThree>());
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
        ctx.v.prefix = proto.prefix;
        ctx.v.suffix = proto.suffix;

        if (proto.has_msg1){
            UnionInnerOneConverter::decoderApply(proto.msg1,ctx.messageContexts.msg1);
            ctx.v.message = std::move(ctx.msg1);
            return true;
        } else if (proto.has_msg2){
            UnionInnerTwoConverter::decoderApply(proto.msg2, ctx.messageContexts.msg2);
            ctx.v.message = std::move(ctx.msg2);
            return true;
        } else if (proto.has_msg3){
            UnionInnerThreeConverter::decoderApply(proto.msg3, ctx.messageContexts.msg3);
            ctx.v.message = std::move(ctx.msg3);
            return true;
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid msg");
            return false;
        }
    }
};