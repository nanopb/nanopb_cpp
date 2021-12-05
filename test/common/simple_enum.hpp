#pragma once

#include "simple_enum.pb.h"

using namespace NanoPb::Converter;

enum class SimpleEnum {
    // Use other values than in proto, to be 100% sure
    Invalid = 100,
    ValueOne = 101,
    ValueTwo = 102
};


class SimpleEnumConverter: public EnumConverter<SimpleEnumConverter, SimpleEnum, PROTO_SimpleEnum> {
public:
    static ProtoType encode(const LocalType& local){
        switch (local) {
            case SimpleEnum::Invalid:
                return PROTO_SimpleEnum_Invalid;
            case SimpleEnum::ValueOne:
                return PROTO_SimpleEnum_ValueOne;
            case SimpleEnum::ValueTwo:
                return PROTO_SimpleEnum_ValueTwo;
        }
        return PROTO_SimpleEnum_Invalid;
    };
    static LocalType decode(const ProtoType& proto){
        switch (proto) {
            case PROTO_SimpleEnum_Invalid:
                return SimpleEnum::Invalid;
            case PROTO_SimpleEnum_ValueOne:
                return SimpleEnum::ValueOne;
            case PROTO_SimpleEnum_ValueTwo:
                return SimpleEnum::ValueTwo;
        }
        return SimpleEnum::Invalid;
    };
};