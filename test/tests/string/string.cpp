#include "tests_common.h"
#include "string.pb.h"

using namespace NanoPb::Converter;

struct LOCAL_TestMessage {
    std::string str;

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return str == rhs.str;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

class TestMessageConverter : public BaseMessageConverter<
        TestMessageConverter,
        LOCAL_TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    static ProtoType encoderInit(const Context& ctx) {
        return ProtoType{
                .str = StringConverter::encoder(ctx.str)
        };
    }

    static ProtoType decoderInit(Context& ctx){
        return ProtoType{
                .str = StringConverter::decoder(ctx.str)
        };
    }

    static bool decoderApply(const ProtoType& proto, Context& ctx){
        return true;
    }
};



int main() {
    int status = 0;

    const LOCAL_TestMessage original(
            {"My super string"}
            );

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);
    return status;
}
