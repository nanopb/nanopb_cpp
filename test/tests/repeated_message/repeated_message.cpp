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

class LocalMessageItemConverter : public AbstractMessageConverter<LocalMessageItemConverter, LocalMessageItem, InnerMessage, &InnerMessage_msg> {
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

class LocalMessageContainerConverter : public AbstractRepeatedMessageConverter<
        LocalMessageContainerConverter,
        LocalMessageContainer,
        LocalMessageItemConverter>
{};

int main() {
    int status = 0;

    const LocalMessageContainer original = {
            LocalMessageItem(1 , "entry_1"),
            LocalMessageItem(2 , "entry_1"),
            LocalMessageItem(UINT32_MAX , "entry_max"),
    };

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        OuterMessage msg = {
                .items = LocalMessageContainerConverter::encoder(&original)
        };

        TEST(pb_encode(&outputStream, &OuterMessage_msg, &msg));
    }

    {
        LocalMessageContainer decoded;

        OuterMessage msg = {
                .items = LocalMessageContainerConverter::decoder(&decoded)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &OuterMessage_msg, &msg));

        TEST(original == decoded);
    }

    return status;
}
