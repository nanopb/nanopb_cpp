#include <utility>
#include <vector>
#include <list>

#include "tests_common.h"
#include "array_string.pb.h"

using namespace NanoPb::Converter;

template <class CONTAINER>
struct LOCAL_TestMessage {
    using ContainerType = CONTAINER;
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

template <class CONTAINER>
class TestMessageConverter : public MessageConverter<
        TestMessageConverter<CONTAINER>,
        LOCAL_TestMessage<CONTAINER>,

        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    using ProtoType = typename TestMessageConverter<CONTAINER>::ProtoType;
    using LocalType = LOCAL_TestMessage<CONTAINER>;
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .values = ArrayStringCallbackConverter<typename LocalType::ContainerType>::encoderCallbackInit(local.values)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .values = ArrayStringCallbackConverter<typename LocalType::ContainerType>::decoderCallbackInit(local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


template <class CONTAINER>
int testRepeated(){
    int status = 0;

    LOCAL_TestMessage<CONTAINER> original({
        "string_1",
        "string_2",
        "string_3"
    });

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter<CONTAINER>>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage<CONTAINER> decoded;

    TEST(NanoPb::decode<TestMessageConverter<CONTAINER>>(inputStream, decoded));

    TEST(original == decoded);
    return status;
}

int main() {
    int status = 0;

    status |= testRepeated<std::vector<std::string>>();
    status |= testRepeated<std::list<std::string>>();

    return status;
}
