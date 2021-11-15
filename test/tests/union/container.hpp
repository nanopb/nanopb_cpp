#pragma once

#include "container_no_union.pb.h"
#include "inner_messages.hpp"


struct LOCAL_UnionContainer {
    int prefix = 0;
    std::unique_ptr<LOCAL_InnerMessage> message;
    int suffix = 0;

    LOCAL_UnionContainer() = default;
    LOCAL_UnionContainer(const LOCAL_UnionContainer&) = delete;
    LOCAL_UnionContainer(LOCAL_UnionContainer&&) = default;
    LOCAL_UnionContainer(int prefix, std::unique_ptr<LOCAL_InnerMessage> &&message, int suffix) :
        prefix(prefix), message(std::move(message)), suffix(suffix)
        {}

    bool operator==(const LOCAL_UnionContainer &rhs) const {
        if (prefix != rhs.prefix || suffix != rhs.suffix){
            return false;
        }
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

    static std::vector<LOCAL_UnionContainer> createTestMessages(){
        std::vector<LOCAL_UnionContainer> ret;
        ret.push_back(LOCAL_UnionContainer(
                15,
                std::unique_ptr<LOCAL_UnionInnerOne>(new LOCAL_UnionInnerOne(111)),
                99
        ));
        ret.push_back(LOCAL_UnionContainer(
                INT_MIN,
                std::unique_ptr<LOCAL_UnionInnerTwo>(
                        new LOCAL_UnionInnerTwo("Message number two")
                ),
                INT_MAX
        ));
        ret.push_back(LOCAL_UnionContainer(
                -9,
                std::unique_ptr<LOCAL_UnionInnerThree>(
                        new LOCAL_UnionInnerThree({0, 1, 2, 3, UINT32_MAX})
                        ),
                        19));
        return ret;
    }
};


using namespace NanoPb::Converter;