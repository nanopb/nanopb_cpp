#pragma once

#include "container.pb.h"
#include "container.hpp"

using namespace NanoPb::Converter;

class UnionContainerNoUnionConverter : public AbstractUnionMessageConverter<
        UnionContainerNoUnionConverter,
        LOCAL_UnionContainer,
        PROTO_UnionContainer,
        &PROTO_UnionContainer_msg>
{
public:
    static ProtoType encoderInit(const LocalType& ctx) {

        ProtoType ret {
            .prefix = ctx.prefix,
            .suffix = ctx.suffix
        };
        NANOPB_CPP_ASSERT(ctx.message);
        if (!ctx.message) {
            return ret;
        }
        switch (ctx.message->getType()) {
            case LOCAL_InnerMessage::Type::UnionInnerOne:
                ret.which_msg = PROTO_UnionContainer_msg1_tag;
                ret.msg.msg1 = UnionInnerOneConverter::encoderInit(*ctx.message->as<LOCAL_UnionInnerOne>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerTwo:
                ret.which_msg = PROTO_UnionContainer_msg2_tag;
                ret.msg.msg2 = UnionInnerTwoConverter::encoderInit(*ctx.message->as<LOCAL_UnionInnerTwo>());
                break;
            case LOCAL_InnerMessage::Type::UnionInnerThree:
                ret.which_msg = PROTO_UnionContainer_msg3_tag;
                ret.msg.msg3 = UnionInnerThreeConverter::encoderInit(*ctx.message->as<LOCAL_UnionInnerThree>());
                break;
        }
        return ret;
    }

    static ProtoType decoderInit(LocalType& ctx){
        return ProtoType{
            .cb_msg = unionDecoderInit(ctx)
        };
    }

    static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &ctx){
        if (field->tag == PROTO_UnionContainer_msg1_tag){
            auto* msg = static_cast<PROTO_UnionInnerOne *>(field->pData);
            ctx.message = std::unique_ptr<LOCAL_UnionInnerOne>(new LOCAL_UnionInnerOne());
            *msg = UnionInnerOneConverter::decoderInit(*ctx.message->as<LOCAL_UnionInnerOne>());
        }
        else if (field->tag == PROTO_UnionContainer_msg2_tag){
            auto* msg = static_cast<PROTO_UnionInnerTwo *>(field->pData);
            ctx.message = std::unique_ptr<LOCAL_UnionInnerTwo>(new LOCAL_UnionInnerTwo());
            *msg = UnionInnerTwoConverter::decoderInit(*ctx.message->as<LOCAL_UnionInnerTwo>());
        }
        else if (field->tag == PROTO_UnionContainer_msg3_tag){
            auto* msg = static_cast<PROTO_UnionInnerThree *>(field->pData);
            ctx.message = std::unique_ptr<LOCAL_UnionInnerThree>(new LOCAL_UnionInnerThree());
            *msg = UnionInnerThreeConverter::decoderInit(*ctx.message->as<LOCAL_UnionInnerThree>());
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid");
        }
        return true;
    }

    static bool decoderApply(const ProtoType& proto, LocalType& ctx){
        ctx.prefix = proto.prefix;
        ctx.suffix = proto.suffix;

        if (proto.which_msg == PROTO_UnionContainer_msg1_tag){
            UnionInnerOneConverter::decoderApply(proto.msg.msg1, *ctx.message->as<LOCAL_UnionInnerOne>());
        }
        else if (proto.which_msg == PROTO_UnionContainer_msg2_tag){
            UnionInnerTwoConverter::decoderApply(proto.msg.msg2, *ctx.message->as<LOCAL_UnionInnerTwo>());
        }
        else if (proto.which_msg == PROTO_UnionContainer_msg3_tag){
            UnionInnerThreeConverter::decoderApply(proto.msg.msg3, *ctx.message->as<LOCAL_UnionInnerThree>());
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid");
        }
        return true;
    }
};