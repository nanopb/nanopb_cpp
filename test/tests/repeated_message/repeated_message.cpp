#include <vector>

#include "tests_common.h"
#include "repeated_message.pb.h"

using namespace NanoPb::Converter;

struct LocalMessageItem {
    uint32_t number = 0;
    std::string text;

    LocalMessageItem() = default; // Default constructor is required
    LocalMessageItem(uint32_t number, const std::string &text) : number(number), text(text) {}

    bool operator==(const LocalMessageItem &rhs) const {
        return number == rhs.number &&
               text == rhs.text;
    }

    bool operator!=(const LocalMessageItem &rhs) const {
        return !(rhs == *this);
    }
};

using LocalMessageContainer = std::vector<LocalMessageItem>;

class RepeatedMessageContainerConverter : public AbstractRepeatedMessageConverter<
        RepeatedMessageContainerConverter,
        LocalMessageContainer,
        InnerMessage,
        &InnerMessage_msg
        >
{
private:
    friend class AbstractRepeatedMessageConverter;

    static ProtoEntryType _encoderInitializer(const LocalEntryType& item){
        return ProtoEntryType{
            .number= item.number,
            .text = StringConverter::encoder(&item.text)
        };
    }

    static ProtoEntryType _decoderInitializer(LocalEntryType& item){
        return ProtoEntryType{
            .number = item.number,
            .text = StringConverter::decoder(&item.text)
        };
    }

    static bool _decoderApply(const ProtoEntryType& protoEntry, LocalEntryType& localEntry){
        localEntry.number = protoEntry.number;
        return true;
    }

};

int main() {
    int status = 0;

    const LocalMessageContainer original = {
            LocalMessageItem(1 , "entry_1"),
            LocalMessageItem(2 , "entry_1"),
            LocalMessageItem(UINT32_MAX , "entry_max"),
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
