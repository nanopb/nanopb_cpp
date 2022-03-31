#include "tests_common.h"

#include "bytes.pb.h"

using namespace NanoPb::Converter;

struct TestMessage {
    std::string data;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;

    bool operator==(const TestMessage &rhs) const {
        return data == rhs.data;
    }

    bool operator!=(const TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public MessageConverter<
        TestMessageConverter,
        TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .data = BytesConverter::encoderCallbackInit(local.data)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .data = BytesConverter::decoderCallbackInit(local.data)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};

int main() {
    int status = 0;

    const TestMessage original(
            {"My super string"}
    );

    NanoPb::StringOutputStream outputStream;

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}