#include <float.h>

#include <map>

#include "tests_common.h"
#include "simple_enum.hpp"
#include "inner_message.hpp"
#include "map.pb.h"

using namespace NanoPb::Converter;

template <class CONTAINER>
struct TestMessage {
    using ContainerType = CONTAINER;

    ContainerType values;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;
    TestMessage(ContainerType&& values) : values(std::move(values)) {}

    bool operator==(const TestMessage &rhs) const {
        return values == rhs.values;
    }

    bool operator!=(const TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

template <class KEY_CONVERTER, class VALUE_CONVERTER,
        class CONTAINER,
        class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG,
        class PROTO_PAIR_TYPE, const pb_msgdesc_t* PROTO_PAIR_TYPE_MSG
        >
class TestMessageConverter : public MessageConverter<
        TestMessageConverter<KEY_CONVERTER, VALUE_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
        TestMessage<CONTAINER>,
        PROTO_TYPE,
        PROTO_TYPE_MSG>
{
public:
    using ProtoType = typename TestMessageConverter<KEY_CONVERTER, VALUE_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>::ProtoType;
    using LocalType = TestMessage<CONTAINER>;
private:
    using ContainerType = typename LocalType::ContainerType;

    using ValuesConverter = MapConverter<
            KEY_CONVERTER,
            VALUE_CONVERTER,
            ContainerType,
            PROTO_PAIR_TYPE,
            PROTO_PAIR_TYPE_MSG>;
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .values = ValuesConverter::encoderCallbackInit(local.values)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .values = ValuesConverter::decoderCallbackInit(local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


template <class CONVERTER, class CONTAINER>
bool testMap(CONTAINER&& values){
    const TestMessage<CONTAINER> original(std::move(values));
    
    NanoPb::StringOutputStream outputStream;

    if (!NanoPb::encode<CONVERTER>(outputStream, original))
        return false;

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    TestMessage<CONTAINER> decoded;

    if(!NanoPb::decode<CONVERTER>(inputStream, decoded))
        return false;

    return original == decoded;
}

#define TEST_MAP(KEY_TYPE, VALUE_TYPE, MAP_TYPE, VALUES)                                \
    {                                                                                   \
     using Converter = TestMessageConverter<                                            \
            CONCAT(KEY_TYPE,Converter), CONCAT(VALUE_TYPE,Converter),                   \
            MAP_TYPE,                                                                   \
            CONCAT4(PROTO_Map_,KEY_TYPE,_,VALUE_TYPE),                                  \
            &CONCAT5(PROTO_Map_,KEY_TYPE,_,VALUE_TYPE,_msg),                            \
            CONCAT5(PROTO_Map_,KEY_TYPE,_,VALUE_TYPE,_ValuesEntry),                     \
            &CONCAT5(PROTO_Map_,KEY_TYPE,_,VALUE_TYPE,_ValuesEntry_msg)                 \
    >;                                                                                  \
    bool CONCAT3(result,KEY_TYPE,VALUE_TYPE) = testMap<Converter, MAP_TYPE> (VALUES);    \
    TEST(CONCAT3(result,KEY_TYPE,VALUE_TYPE));                                          \
    }


#define _ , // Comma can't be passed to macro

int main() {
    int status = 0;

    // 32 but types
    TEST_MAP(Int32, Int32, std::map<int32_t _ int32_t>, {
        {INT32_MIN _ INT32_MAX} _
        {INT32_MAX _ INT32_MIN}
    });
    TEST_MAP(SInt32, SInt32, std::map<int32_t _ int32_t>, {
        {INT32_MIN _ INT32_MAX} _
        {INT32_MAX _ INT32_MIN}
    });
    TEST_MAP(UInt32, UInt32, std::map<uint32_t _ uint32_t>, {
        {0 _ UINT32_MAX} _
        {UINT32_MAX _ 0}
    });
    TEST_MAP(Fixed32, Fixed32, std::map<uint32_t _ uint32_t>, {
        {0 _ UINT32_MAX} _
        {UINT32_MAX _ 0}
    });
    TEST_MAP(SFixed32, SFixed32, std::map<int32_t _ int32_t>, {
        {INT32_MIN _ INT32_MAX} _
        {INT32_MAX _ INT32_MIN}
    });
    TEST_MAP(Int32, Float, std::map<int32_t _ float>, {
        {INT32_MIN _ FLT_MIN} _
        {INT32_MAX _ FLT_MAX}
    });

// TODO: std::map<xxx,bool> has specific implementation which is not compatible with standard behavior
//    TEST_MAP(Bool, Bool, std::map<int32_t _ bool>, {
//        {true _ false} _
//        {false _ true}
//    });

    // string/bytes

    TEST_MAP(String, String, std::map<std::string _ std::string>, {
        {"key 1" _ "value 1"} _
        {"key 2" _ "value 2"}
    });
    TEST_MAP(Int32, Bytes, std::map<int32_t _ std::string>, {
        {INT32_MIN _ "value 1"} _
        {INT32_MAX _ "value 2"}
    });

    // 64 bit types
#ifndef PB_WITHOUT_64BIT
    TEST_MAP(Int64, Int64, std::map<int64_t _ int64_t>, {
        {INT64_MIN _ INT64_MAX} _
        {INT64_MAX _ INT64_MIN}
    });
    TEST_MAP(SInt64, SInt64, std::map<int64_t _ int64_t>, {
        {INT64_MIN _ INT64_MAX} _
        {INT64_MAX _ INT64_MIN}
    });
    TEST_MAP(UInt64, UInt64, std::map<uint64_t _ uint64_t>, {
        {0 _ UINT64_MAX} _
        {UINT64_MAX _ 0}
    });
    TEST_MAP(Fixed64, Fixed64, std::map<uint64_t _ uint64_t>, {
        {0 _ UINT64_MAX} _
        {UINT64_MAX _ 0}
    });
    TEST_MAP(SFixed64, SFixed64, std::map<int64_t _ int64_t>, {
        {INT64_MIN _ INT64_MAX} _
        {INT64_MAX _ INT64_MIN}
    });
    TEST_MAP(Int32, Double, std::map<int32_t _ double>, {
        {INT32_MIN _ DBL_MIN} _
        {INT32_MAX _ DBL_MAX}
    });
#endif

    // Complicated types

    TEST_MAP(Int32, SimpleEnum, std::map<int32_t _ SimpleEnum>, {
        {0 _ SimpleEnum::Invalid } _
        {INT32_MIN _ SimpleEnum::ValueOne } _
        {INT32_MAX _ SimpleEnum::ValueTwo }
    });
    TEST_MAP(String, SimpleEnum, std::map<std::string _ SimpleEnum>, {
        {"key 1" _ SimpleEnum::Invalid } _
        {"key 2" _ SimpleEnum::ValueOne } _
        {"key 3" _ SimpleEnum::ValueTwo }
    });

    {
        std::map<int32_t, InnerMessage> mapByInt;
        int i = 0;
        for (auto &&msg: InnerMessage::createTestMessages<std::vector<InnerMessage>>()) {
            mapByInt.emplace(i, std::move(msg));
            i++;
        }
        TEST_MAP(Int32, InnerMessage, std::map<int32_t _ InnerMessage>, std::move(mapByInt));
    }
    {
        int i = 0;
        std::map<std::string, InnerMessage> mapByString;
        for (auto &&msg: InnerMessage::createTestMessages<std::vector<InnerMessage>>()) {
            std::string key = "key " + std::to_string(i);
            mapByString.emplace(key, std::move(msg));
            i++;
        }
        TEST_MAP(String, InnerMessage, std::map<std::string _ InnerMessage>, std::move(mapByString));
    }

    return status;
}
