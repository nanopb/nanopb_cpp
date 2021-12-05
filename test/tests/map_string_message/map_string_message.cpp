#include <map>

#include "tests_common.h"
#include "inner_message.hpp"
#include "map_string_message.pb.h"

using namespace NanoPb::Converter;

struct TestMessage {
    using MapType = std::map<std::string, InnerMessage>;
    MapType items;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;
    TestMessage(MapType &&items) : items(std::move(items)) {}

    bool operator==(const TestMessage &rhs) const {
        return items == rhs.items;
    }

    bool operator!=(const TestMessage &rhs) const {
        return !(rhs == *this);
    }

    static std::vector<TestMessage> createTestMessages() {
        std::vector<TestMessage> ret;
        auto innerMessages = InnerMessage::createTestMessages<std::vector<InnerMessage>>();

        TestMessage::MapType items;

        uint32_t key = 0;
        for (auto& m: innerMessages){
            items.emplace("key_" + std::to_string(key), std::move(m));
            key++;
        }

        ret.push_back(TestMessage(std::move(items)));

        return ret;
    }
};

class TestMessageConverter : public MessageConverter<
        TestMessageConverter,
        TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
private:
    using ItemsConverter = MapConverter<
            StringConverter,
            InnerMessageConverter,
            TestMessage::MapType,
            PROTO_TestMessage_ItemsEntry,
            &PROTO_TestMessage_ItemsEntry_msg>;

public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .items = ItemsConverter::encoderCallbackInit(local.items)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .items = ItemsConverter::decoderCallbackInit(local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};


int main() {
    int status = 0;

    const auto messages = TestMessage::createTestMessages();

    for (auto& original : messages){

        NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

        TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TestMessage decoded;

        TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

        TEST(original == decoded);
    }

    return status;
}
