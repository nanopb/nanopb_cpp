#include <utility>
#include <vector>
#include <list>

#include "tests_common.h"
#include "repeated_signed.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    using ContainerType = std::vector<int64_t>;
    ContainerType values;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(ContainerType values) : values(std::move(values)) {}

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return values == rhs.values;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public AbstractMessageConverter<TestMessageConverter, LOCAL_TestMessage, PROTO_TestMessage , &PROTO_TestMessage_msg> {
private:
    friend class AbstractMessageConverter;

private:
    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .values = RepeatedUnsignedConverter<LOCAL_TestMessage::ContainerType>::encoder(local.values)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
                .values = RepeatedUnsignedConverter<LOCAL_TestMessage::ContainerType>::decoder(local.values)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


template <class CONTAINER>
int testRepeated(const typename CONTAINER::value_type minValue, const typename CONTAINER::value_type maxValue){
    int status = 0;

    LOCAL_TestMessage original({minValue, 0, 1 , 2, 3, 4, 5, maxValue});

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);
    return status;
}

int main() {
    int status = 0;

    status |= testRepeated<std::vector<int64_t>>(INT64_MIN, INT64_MAX);
    status |= testRepeated<std::list<int64_t>>(INT64_MIN, INT64_MAX);

    status |= testRepeated<std::vector<int32_t>>(INT32_MIN, INT32_MAX);
    status |= testRepeated<std::list<int32_t>>(INT32_MIN, INT32_MAX);

    return status;
}
