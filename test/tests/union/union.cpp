#include <vector>
#include <memory>

#include "tests_common.h"
#include "converter_no_union.hpp"

using namespace NanoPb::Converter;

template <class UNION_CONVERTER>
int test_standard(const LOCAL_UnionContainer& original){
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<UNION_CONVERTER>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionContainer decoded;

    TEST(NanoPb::decode<UNION_CONVERTER>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}

// *** ENCODE ONLY union message, without prefix/suffix ***
template <class UNION_CONVERTER>
int test_manual_encode(const LOCAL_UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    switch (original.message->getType()) {
        case LOCAL_InnerMessage::Type::UnionInnerOne: {
            auto& local = *original.message->as<LOCAL_UnionInnerOne>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerOneConverter>(outputStream, local, &PROTO_UnionContainerNoUnion_msg));
            break;
        }
        case LOCAL_InnerMessage::Type::UnionInnerTwo: {
            auto& local = *original.message->as<LOCAL_UnionInnerTwo>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerTwoConverter>(outputStream, local, &PROTO_UnionContainerNoUnion_msg));
            break;
        }
        case LOCAL_InnerMessage::Type::UnionInnerThree: {
            auto& local = *original.message->as<LOCAL_UnionInnerThree>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerThreeConverter>(outputStream, local, &PROTO_UnionContainerNoUnion_msg));
            break;
        }
    }

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionContainer decoded;

    TEST(NanoPb::decode<UNION_CONVERTER>(inputStream, decoded));

    // Compare only message
    TEST(*original.message ==* decoded.message);

    return status;
}

template <class UNION_CONVERTER>
int test_manual_decode(const LOCAL_UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    // Encode with standard way
    TEST(NanoPb::encode<UNION_CONVERTER>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    const pb_msgdesc_t* type = NanoPb::decodeUnionMessageType(inputStream, &PROTO_UnionContainerNoUnion_msg);

    TEST(type);
    if (type){
        if (type == UnionInnerOneConverter::getMsgType())
        {
            LOCAL_UnionInnerOne decodedMessage;
            TEST(NanoPb::decodeSubMessage<UnionInnerOneConverter>(inputStream, decodedMessage))
        }
        else if (type == UnionInnerTwoConverter::getMsgType())
        {
            LOCAL_UnionInnerTwo decodedMessage;
            TEST(NanoPb::decodeSubMessage<UnionInnerTwoConverter>(inputStream, decodedMessage))
        }
        else if (type == UnionInnerThreeConverter::getMsgType())
        {
            LOCAL_UnionInnerThree decodedMessage;
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

    const auto messages = LOCAL_UnionContainer::createTestMessages();

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
