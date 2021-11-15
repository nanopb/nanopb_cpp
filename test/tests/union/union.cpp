#include <vector>
#include <memory>

#include "tests_common.h"
#include "union_message.hpp"

using namespace NanoPb::Converter;


int test_standard(const LOCAL_UnionContainer& original){
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<UnionContainerConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionContainer decoded;

    TEST(NanoPb::decode<UnionContainerConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}

int test_manual_encode(const LOCAL_UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    switch (original.message->getType()) {
        case LOCAL_InnerMessage::Type::UnionInnerOne: {
            auto& local = *original.message->as<LOCAL_UnionInnerOne>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerOneConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
        case LOCAL_InnerMessage::Type::UnionInnerTwo: {
            auto& local = *original.message->as<LOCAL_UnionInnerTwo>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerTwoConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
        case LOCAL_InnerMessage::Type::UnionInnerThree: {
            auto& local = *original.message->as<LOCAL_UnionInnerThree>();
            TEST(NanoPb::encodeUnionMessage<UnionInnerThreeConverter>(outputStream, local, &PROTO_UnionContainer_msg));
            break;
        }
    }

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionContainer decoded;

    TEST(NanoPb::decode<UnionContainerConverter>(inputStream, decoded));

    TEST(original == decoded);


    return status;
}

int test_manual_decode(const LOCAL_UnionContainer& original) {
    int status = 0;

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    // Encode with standard way
    TEST(NanoPb::encode<UnionContainerConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    uint32_t tag;

    bool tagDecodeSuccess = NanoPb::decodeTag(inputStream, tag);

    TEST(tagDecodeSuccess);
    if (tagDecodeSuccess){
        switch (original.message->getType()) {
            case LOCAL_InnerMessage::Type::UnionInnerOne: {
                TEST(tag == PROTO_UnionContainer_msg1_tag);

                LOCAL_UnionInnerOne decodedMessage;
                TEST(NanoPb::decodeSubMessage<UnionInnerOneConverter>(inputStream, decodedMessage))
                break;
            }
            case LOCAL_InnerMessage::Type::UnionInnerTwo: {
                TEST(tag == PROTO_UnionContainer_msg2_tag);

                LOCAL_UnionInnerTwo decodedMessage;
                TEST(NanoPb::decodeSubMessage<UnionInnerTwoConverter>(inputStream, decodedMessage))
                break;
            }
            case LOCAL_InnerMessage::Type::UnionInnerThree: {
                TEST(tag == PROTO_UnionContainer_msg3_tag);

                LOCAL_UnionInnerThree decodedMessage;
                TEST(NanoPb::decodeSubMessage<UnionInnerThreeConverter>(inputStream, decodedMessage))
                break;
            }
        }
    }

    return status;
}

int main() {
    int status = 0;

    const auto messages = LOCAL_UnionContainer::createTestMessages();

    for (auto& original : messages){
        status |= test_standard(original);
        status |= test_manual_encode(original);
        status |= test_manual_decode(original);
    }

    return status;
}
