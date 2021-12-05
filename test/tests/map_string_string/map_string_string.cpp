#include <map>

#include "tests_common.h"
#include "map_string_string.pb.h"

using namespace NanoPb::Converter;

struct TestMessage {
    using SimpleMapType = std::map<std::string, std::string>;
    SimpleMapType items;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;

    bool operator==(const TestMessage &rhs) const {
        return items == rhs.items;
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
private:
    using ValuesConverter = MapConverter<
            StringConverter,
            StringConverter,
            TestMessage::SimpleMapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>;

public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .items = ValuesConverter::encoderCallbackInit(local.items)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .items = ValuesConverter::decoderCallbackInit(local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


int main() {
    int status = 0;

    const TestMessage original = {
            .items = {
                    {"key_1", "value_1"},
                    {"key_2", "value_2"}
            }
    };

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}
