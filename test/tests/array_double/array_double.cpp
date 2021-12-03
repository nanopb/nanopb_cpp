#include <utility>
#include <vector>
#include <list>

#include "tests_common.h"
#include "array_double.pb.h"

#ifndef PB_WITHOUT_64BIT


using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using ContainerType = std::vector<double>;
    ContainerType values;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(const LOCAL_TestMessage&) = delete;
    LOCAL_TestMessage(LOCAL_TestMessage&&) = default;
    LOCAL_TestMessage(ContainerType values) : values(std::move(values)) {}

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return values == rhs.values;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public MessageConverter<
        TestMessageConverter,
        LOCAL_TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .values = ArrayDoubleCallbackConverter<LOCAL_TestMessage::ContainerType>::encoderCallbackInit(local.values)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .values = ArrayDoubleCallbackConverter<LOCAL_TestMessage::ContainerType>::decoderCallbackInit(local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


template <class CONTAINER>
int testRepeated(){
    int status = 0;

    LOCAL_TestMessage original({-(float)(INT64_MAX), -100.45f, 0, 1.3f , 212.f, 544.4f, (float)INT64_MAX});

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);
    return status;
}
#endif

int main() {
    int status = 0;

#ifndef PB_WITHOUT_64BIT
    status |= testRepeated<std::vector<double>>();
    status |= testRepeated<std::list<double>>();
#endif

    return status;
}
