#include <vector>
#include <list>

#include "tests_common.h"
#include "inner_message.hpp"
#include "array_message.pb.h"

using namespace NanoPb::Converter;

template <class CONTAINER>
struct LOCAL_TestMessage {
    using ItemsContainer = CONTAINER;

    int32_t number = 0;
    ItemsContainer items;

    LOCAL_TestMessage() = default;
    LOCAL_TestMessage(const LOCAL_TestMessage&) = delete;
    LOCAL_TestMessage(LOCAL_TestMessage&&) = default;

    LOCAL_TestMessage(int32_t number, ItemsContainer &&items) : number(number), items(std::move(items)) {}

    bool operator==(const LOCAL_TestMessage &rhs) const {
        return number == rhs.number &&
               items == rhs.items;
    }

    bool operator!=(const LOCAL_TestMessage &rhs) const {
        return !(rhs == *this);
    }
};

template <class CONTAINER>
class OuterMessageConverter : public MessageConverter<
        OuterMessageConverter<CONTAINER>,
        LOCAL_TestMessage<CONTAINER>,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    using ProtoType = typename OuterMessageConverter<CONTAINER>::ProtoType;
    using LocalType = LOCAL_TestMessage<CONTAINER>;
private:
    class ItemsConverter : public ArrayConverter<
            InnerMessageConverter,
            typename LOCAL_TestMessage<CONTAINER>::ItemsContainer
            >
    {};

public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .number = local.number,
                .items = ItemsConverter::encoderCallbackInit(local.items)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .items = ItemsConverter::decoderCallbackInit(local.items)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};

template <class CONTAINER>
int testRepeated(){
    int status = 0;

    std::vector<LOCAL_TestMessage<CONTAINER>> messages;
    {
        CONTAINER items;
        items.push_back(LOCAL_InnerMessage(1, "entry_1"));
        items.push_back(LOCAL_InnerMessage(2, "entry_1"));
        items.push_back(LOCAL_InnerMessage(UINT32_MAX, "entry_max"));

        messages.push_back(
                LOCAL_TestMessage<CONTAINER>(INT32_MIN, std::move(items))
        );
    }

    for (const LOCAL_TestMessage<CONTAINER>& original : messages) {

        NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

        TEST(NanoPb::encode<OuterMessageConverter<CONTAINER>>(outputStream, original));

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        LOCAL_TestMessage<CONTAINER> decoded;

        TEST(NanoPb::decode<OuterMessageConverter<CONTAINER>>(inputStream, decoded));

        TEST(original == decoded);
    }
    return status;
}


int main() {
    int status = 0;

    status |= testRepeated<std::vector<LOCAL_InnerMessage>>();
    status |= testRepeated<std::list<LOCAL_InnerMessage>>();

    return status;
}
