#include <vector>
#include <memory>

#include "tests_common.h"
#include "converter.hpp"

using namespace NanoPb::Converter;

template <class UNION_CONVERTER>
int test_standard(const UnionContainer& original){
    int status = 0;

    NanoPb::StringOutputStream outputStream;

    TEST(NanoPb::encode<UNION_CONVERTER>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    UnionContainer decoded;

    TEST(NanoPb::decode<UNION_CONVERTER>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}

// *** ENCODE ONLY union message, without prefix/suffix ***
template <class UNION_CONVERTER>
int test_manual_encode(const UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream;

    switch (original.message->getType()) {
        case InnerMessage::Type::UnionInnerOne: {
            auto& local = *original.message->as<UnionInnerOne>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerOneConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
        case InnerMessage::Type::UnionInnerTwo: {
            auto& local = *original.message->as<UnionInnerTwo>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerTwoConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
        case InnerMessage::Type::UnionInnerThree: {
            auto& local = *original.message->as<UnionInnerThree>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerThreeConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
    }

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    UnionContainer decoded;

    TEST(NanoPb::decode<UNION_CONVERTER>(inputStream, decoded));

    // Compare only message
    TEST(*original.message ==* decoded.message);

    return status;
}

template <class UNION_CONVERTER>
int test_manual_decode(const UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream;

    // Encode with standard way
    TEST(NanoPb::encode<UNION_CONVERTER>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    const pb_msgdesc_t* type = NanoPb::decodeUnionMessageType(inputStream, &PROTO_UnionContainer_msg);

    TEST(type);
    if (type){
        if (type == UnionInnerOneConverter::getMsgType())
        {
            UnionInnerOne decodedMessage;
            TEST(NanoPb::decodeSubMessage<UnionInnerOneConverter>(inputStream, decodedMessage))
        }
        else if (type == UnionInnerTwoConverter::getMsgType())
        {
            UnionInnerTwo decodedMessage;
            TEST(NanoPb::decodeSubMessage<UnionInnerTwoConverter>(inputStream, decodedMessage))
        }
        else if (type == UnionInnerThreeConverter::getMsgType())
        {
            UnionInnerThree decodedMessage;
            TEST(NanoPb::decodeSubMessage<UnionInnerThreeConverter>(inputStream, decodedMessage))
        }
        else
        {
            TEST(0&&"Invalid type");
        }
    }

    return status;
}

int main() {
    int status = 0;

    const auto messages = UnionContainer::createTestMessages();

    for (auto& original : messages){
        {
            COMMENT("test_standard, type: %d", (int)original.message->getType());
            status |= test_standard<UnionContainerNoUnionConverter>(original);
            COMMENT("test_manual_encode, type: %d", (int)original.message->getType());
            status |= test_manual_encode<UnionContainerNoUnionConverter>(original);
            COMMENT("test_manual_decode, type: %d", (int)original.message->getType());
            status |= test_manual_decode<UnionContainerNoUnionConverter>(original);
        }
    }

    return status;
}
