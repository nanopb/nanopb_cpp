#include "tests_common.h"
#include "enum.pb.h"

enum class SimpleEnum {
    Invalid = 0,
    ValueOne = 1,
    ValueTwo = 2
};


class SimpleEnumConverter: public NanoPb::Converter::EnumConverter<SimpleEnumConverter, SimpleEnum, PROTO_SimpleEnum> {
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

int testEnumItem(SimpleEnum local, PROTO_SimpleEnum proto){
    int status = 0;

    auto localToProto = SimpleEnumConverter::encode(local);

    TEST(localToProto == proto);

    auto protoToLocal = SimpleEnumConverter::decode(proto);

    TEST(protoToLocal == local);

    return status;
}


int main() {
    int status = 0;

    status |= testEnumItem(SimpleEnum::Invalid,PROTO_SimpleEnum_Invalid);
    status |= testEnumItem(SimpleEnum::ValueOne,PROTO_SimpleEnum_ValueOne);
    status |= testEnumItem(SimpleEnum::ValueTwo,PROTO_SimpleEnum_ValueTwo);


    return status;
}
