#include <float.h>

#include <vector>
#include <list>

#include "tests_common.h"
#include "simple_enum.hpp"
#include "inner_message.hpp"
#include "array.pb.h"

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

template <class ITEM_CONVERTER, class CONTAINER, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
class TestMessageConverter : public MessageConverter<
        TestMessageConverter<ITEM_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG>,
        TestMessage<CONTAINER>,
        PROTO_TYPE,
        PROTO_TYPE_MSG>
{
public:
    using ProtoType = typename TestMessageConverter<ITEM_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG>::ProtoType;
    using LocalType = TestMessage<CONTAINER>;
private:
    using ContainerType = typename LocalType::ContainerType;
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .values = ArrayConverter<ITEM_CONVERTER, ContainerType>::encoderCallbackInit(local.values)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .values = ArrayConverter<ITEM_CONVERTER, ContainerType>::decoderCallbackInit(local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


template <class ITEM_CONVERTER, class CONTAINER, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
bool testRepeated(CONTAINER&& values){
    const TestMessage<CONTAINER> original(std::move(values));

    NanoPb::StringOutputStream outputStream;

    if (!NanoPb::encode<TestMessageConverter<ITEM_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG>>(outputStream, original))
        return false;

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    TestMessage<CONTAINER> decoded;

    if(!NanoPb::decode<TestMessageConverter<ITEM_CONVERTER, CONTAINER, PROTO_TYPE, PROTO_TYPE_MSG>>(inputStream, decoded))
        return false;

    return original == decoded;
}

#define TEST_ARRAY(PROTO_TYPE, TYPE, VALUES)  \
    {                                               \
    bool CONCAT(result,PROTO_TYPE) = testRepeated<                     \
        CONCAT(PROTO_TYPE,Converter),       \
        TYPE,                                 \
        CONCAT(PROTO_Repeated_,PROTO_TYPE),         \
        &CONCAT3(PROTO_Repeated_,PROTO_TYPE,_msg)   \
    >(VALUES);                                      \
    TEST(CONCAT(result,PROTO_TYPE));                                   \
    }

#define _ , // Comma can't be passed to macro

int main() {
    int status = 0;

    // 32 bit types

    TEST_ARRAY(Int32,   std::vector<int32_t>,   {INT32_MIN _ 0 _ INT32_MAX});
    TEST_ARRAY(Int32,   std::list<int32_t>,     {INT32_MIN _ 0 _ INT32_MAX});

    TEST_ARRAY(SInt32,  std::vector<int32_t>,   {INT32_MIN _ 0 _ INT32_MAX});
    TEST_ARRAY(SInt32,  std::list<int32_t>,     {INT32_MIN _ 0 _ INT32_MAX});

    TEST_ARRAY(UInt32,  std::vector<uint32_t>,  {0 _ UINT32_MAX});
    TEST_ARRAY(UInt32,  std::list<uint32_t>,    {0 _ UINT32_MAX});

    TEST_ARRAY(Fixed32, std::vector<uint32_t>,  {0 _ UINT32_MAX});
    TEST_ARRAY(Fixed32, std::list<uint32_t>,    {0 _ UINT32_MAX});

    TEST_ARRAY(SFixed32,std::vector<int32_t>,   {INT32_MIN _ 0 _ INT32_MAX});
    TEST_ARRAY(SFixed32,std::list<int32_t>,     {INT32_MIN _ 0 _ INT32_MAX});

    TEST_ARRAY(Float,   std::vector<float>,     {FLT_MIN _ 0 _ FLT_MAX});
    TEST_ARRAY(Float,   std::list<float>,       {FLT_MIN _ 0 _ FLT_MAX});

// TODO: std::vector<bool> has specific implementation which is not compatible with standard behavior
//    TEST_ARRAY(Bool,    std::vector<bool>,      {true _ false});
//    TEST_ARRAY(Bool,    std::list<bool>,        {true _ false});

    // string/bytes

    TEST_ARRAY(String,  std::vector<std::string>,{"One" _ "Two"});
    TEST_ARRAY(String,  std::list<std::string>, {"One" _ "Two"});

    TEST_ARRAY(Bytes,   std::vector<std::string>,{"One" _ "Two"});
    TEST_ARRAY(Bytes,   std::list<std::string>, {"One" _ "Two"});

    // 64 bit types

#ifndef PB_WITHOUT_64BIT
    TEST_ARRAY(Int64,   std::vector<int64_t>,   {INT64_MIN _ 0 _ INT64_MAX});
    TEST_ARRAY(Int64,   std::list<int64_t>,     {INT64_MIN _ 0 _ INT64_MAX});

    TEST_ARRAY(SInt64,  std::vector<int64_t>,   {INT64_MIN _ 0 _ INT64_MAX});
    TEST_ARRAY(SInt64,  std::list<int64_t>,     {INT64_MIN _ 0 _ INT64_MAX});

    TEST_ARRAY(UInt64,  std::vector<uint64_t>,  {0 _ UINT64_MAX});
    TEST_ARRAY(UInt64,  std::list<uint64_t>,    {0 _ UINT64_MAX});

    TEST_ARRAY(Fixed64, std::vector<uint64_t>,  {0 _ UINT64_MAX});
    TEST_ARRAY(Fixed64, std::list<uint64_t>,    {0 _ UINT64_MAX});

    TEST_ARRAY(SFixed64,std::vector<int64_t>,   {INT64_MIN _ 0 _ INT64_MAX});
    TEST_ARRAY(SFixed64,std::list<int64_t>,     {INT64_MIN _ 0 _ INT64_MAX});

    TEST_ARRAY(Double,  std::vector<double>,    {DBL_MIN _ 0 _ DBL_MAX});
    TEST_ARRAY(Double,  std::list<double>,      {DBL_MIN _ 0 _ DBL_MAX});
#endif

    // Complicated types

    TEST_ARRAY(SimpleEnum,  std::vector<SimpleEnum>,      {SimpleEnum::Invalid _ SimpleEnum::ValueOne _ SimpleEnum::ValueTwo});
    TEST_ARRAY(SimpleEnum,  std::list<SimpleEnum>,      {SimpleEnum::Invalid _ SimpleEnum::ValueOne _ SimpleEnum::ValueTwo});

    TEST_ARRAY(InnerMessage,  std::vector<InnerMessage>, InnerMessage::createTestMessages<std::vector<InnerMessage>>());
    TEST_ARRAY(InnerMessage,  std::list<InnerMessage>, InnerMessage::createTestMessages<std::list<InnerMessage>>());


    return status;
}
