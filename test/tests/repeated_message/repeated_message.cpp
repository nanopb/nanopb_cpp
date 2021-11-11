#include <vector>

#include "tests_common.h"
#include "repeated_message.pb.h"

using namespace NanoPb::Converter;

struct LocalInnerMessage {
    uint32_t number = 0;
    std::string text;

    LocalInnerMessage() = default; // Default constructor is required
    LocalInnerMessage(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const LocalInnerMessage &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const LocalInnerMessage &rhs) const {
        return !(rhs == *this);
    }
};

using LocalMessageContainer = std::vector<LocalInnerMessage>;

class LocalMessageItemConverter : public AbstractMessageConverter<LocalMessageItemConverter, LocalInnerMessage, InnerMessage, &InnerMessage_msg> {
private:
    friend class AbstractMessageConverter;

    static ProtoType _encoderInit(const LocalType& local) {
        return ProtoType{
                .number = local.number,
                .text = StringConverter::encoder(&local.text)
        };
    }

    static ProtoType _decoderInit(LocalType& local){
        return ProtoType{
            .text = StringConverter::decoder(&local.text)
        };
    }

    static bool _decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};

class RepeatedMessageContainerConverter : public AbstractRepeatedMessageConverter<
        RepeatedMessageContainerConverter,
        LocalMessageContainer,
        InnerMessage,
        &InnerMessage_msg
        >
{
private:
    friend class AbstractRepeatedMessageConverter;

    static ProtoEntryType _encoderInit(const LocalEntryType& item){
        return LocalMessageItemConverter::encoderInit(item);
    }

    static ProtoEntryType _decoderInit(LocalEntryType& item){
        return LocalMessageItemConverter::decoderInit(item);
    }

    static bool _decoderApply(const ProtoEntryType& protoEntry, LocalEntryType& localEntry){
        return LocalMessageItemConverter::decoderApply(protoEntry, localEntry);
    }

};

int main() {
    int status = 0;

    const LocalMessageContainer original = {
            LocalInnerMessage(1 , "entry_1"),
            LocalInnerMessage(2 , "entry_1"),
            LocalInnerMessage(UINT32_MAX , "entry_max"),
    };

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        RepeatedMessageContainerMessage msg = {
                .items = RepeatedMessageContainerConverter::encoder(&original)
        };

        TEST(pb_encode(&outputStream, &RepeatedMessageContainerMessage_msg, &msg));
    }

    {
        LocalMessageContainer decoded;

        RepeatedMessageContainerMessage msg = {
                .items = RepeatedMessageContainerConverter::decoder(&decoded)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &RepeatedMessageContainerMessage_msg, &msg));

        TEST(original == decoded);
    }

    return status;
}
