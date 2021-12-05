#pragma once

#include "container.pb.h"
#include "inner_messages.hpp"


struct UnionContainer {
    int prefix = 0;
    std::unique_ptr<InnerMessage> message;
    int suffix = 0;

    UnionContainer() = default;
    UnionContainer(const UnionContainer&) = delete;
    UnionContainer(UnionContainer&&) = default;
    UnionContainer(int prefix, std::unique_ptr<InnerMessage> &&message, int suffix) :
        prefix(prefix), message(std::move(message)), suffix(suffix)
        {}

    bool operator==(const UnionContainer &rhs) const {
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
            case InnerMessage::Type::UnionInnerOne:
                return *rhs.message->as<UnionInnerOne>() == *message->as<UnionInnerOne>();
            case InnerMessage::Type::UnionInnerTwo:
                return *rhs.message->as<UnionInnerTwo>() == *message->as<UnionInnerTwo>();
            case InnerMessage::Type::UnionInnerThree:
                return *rhs.message->as<UnionInnerThree>() == *message->as<UnionInnerThree>();
        }
        NANOPB_CPP_ASSERT(0&&"Invalid type");
        return false;
    }

    bool operator!=(const UnionContainer &rhs) const {
        return !(rhs == *this);
    }

    static std::vector<UnionContainer> createTestMessages(){
        std::vector<UnionContainer> ret;
        ret.push_back(UnionContainer(
                15,
                std::unique_ptr<UnionInnerOne>(new UnionInnerOne(111)),
                99
        ));
        ret.push_back(UnionContainer(
                INT_MIN,
                std::unique_ptr<UnionInnerTwo>(
                        new UnionInnerTwo("Message number two")
                ),
                INT_MAX
        ));
        ret.push_back(UnionContainer(
                -9,
                std::unique_ptr<UnionInnerThree>(
                        new UnionInnerThree({0, 1, 2, 3, UINT32_MAX})
                        ),
                        19));
        return ret;
    }
};


using namespace NanoPb::Converter;