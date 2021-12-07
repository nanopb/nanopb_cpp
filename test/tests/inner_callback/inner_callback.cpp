#include <float.h>

#include "tests_common.h"
#include "inner_message.hpp"
#include "nanopb_cpp.h"
#include "inner_callback.pb.h"

using namespace NanoPb::Type;

struct TestMessage {
    InnerMessage inner;

    TestMessage() = default; // Default constructor is required
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&& other) = default;
    TestMessage(InnerMessage&& inner) : inner(std::move(inner)){};

    bool operator==(const TestMessage &rhs) const {
        return inner == rhs.inner;
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
                .inner = InnerMessageConverter::encoderCallbackInit(local.inner)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .inner = InnerMessageConverter::decoderCallbackInit(local.inner)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};



int main() {
    int status = 0;

    for (auto &&msg: InnerMessage::createTestMessages<std::vector<InnerMessage>>()) {

        TestMessage original(std::move(msg));

        NanoPb::StringOutputStream outputStream;

        TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TestMessage decoded;

        TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

        TEST(original == decoded);
    }

    return status;
}
