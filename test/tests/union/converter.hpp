#pragma once

#include "container.pb.h"
#include "container.hpp"

using namespace NanoPb::Converter;

class UnionContainerNoUnionConverter : public UnionMessageConverter<
        UnionContainerNoUnionConverter,
        UnionContainer,
        PROTO_UnionContainer,
        &PROTO_UnionContainer_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {

        ProtoType ret {
            .prefix = local.prefix,
            .suffix = local.suffix
        };
        NANOPB_CPP_ASSERT(local.message);
        if (!local.message) {
            return ret;
        }
        switch (local.message->getType()) {
            case InnerMessage::Type::UnionInnerOne:
                ret.which_msg = PROTO_UnionContainer_msg1_tag;
                ret.msg.msg1 = UnionInnerOneConverter::encoderInit(*local.message->as<UnionInnerOne>());
                break;
            case InnerMessage::Type::UnionInnerTwo:
                ret.which_msg = PROTO_UnionContainer_msg2_tag;
                ret.msg.msg2 = UnionInnerTwoConverter::encoderInit(*local.message->as<UnionInnerTwo>());
                break;
            case InnerMessage::Type::UnionInnerThree:
                ret.which_msg = PROTO_UnionContainer_msg3_tag;
                ret.msg.msg3 = UnionInnerThreeConverter::encoderInit(*local.message->as<UnionInnerThree>());
                break;
        }
        return ret;
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
            .cb_msg = unionDecoderInit(local)
        };
    }

    static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
        if (field->tag == PROTO_UnionContainer_msg1_tag){
            auto* msg = static_cast<PROTO_UnionInnerOne *>(field->pData);
            local.message = std::unique_ptr<UnionInnerOne>(new UnionInnerOne());
            *msg = UnionInnerOneConverter::decoderInit(*local.message->as<UnionInnerOne>());
        }
        else if (field->tag == PROTO_UnionContainer_msg2_tag){
            auto* msg = static_cast<PROTO_UnionInnerTwo *>(field->pData);
            local.message = std::unique_ptr<UnionInnerTwo>(new UnionInnerTwo());
            *msg = UnionInnerTwoConverter::decoderInit(*local.message->as<UnionInnerTwo>());
        }
        else if (field->tag == PROTO_UnionContainer_msg3_tag){
            auto* msg = static_cast<PROTO_UnionInnerThree *>(field->pData);
            local.message = std::unique_ptr<UnionInnerThree>(new UnionInnerThree());
            *msg = UnionInnerThreeConverter::decoderInit(*local.message->as<UnionInnerThree>());
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid");
        }
        return true;
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        local.prefix = proto.prefix;
        local.suffix = proto.suffix;

        if (proto.which_msg == PROTO_UnionContainer_msg1_tag){
            UnionInnerOneConverter::decoderApply(proto.msg.msg1, *local.message->as<UnionInnerOne>());
        }
        else if (proto.which_msg == PROTO_UnionContainer_msg2_tag){
            UnionInnerTwoConverter::decoderApply(proto.msg.msg2, *local.message->as<UnionInnerTwo>());
        }
        else if (proto.which_msg == PROTO_UnionContainer_msg3_tag){
            UnionInnerThreeConverter::decoderApply(proto.msg.msg3, *local.message->as<UnionInnerThree>());
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid");
        }
        return true;
    }
};