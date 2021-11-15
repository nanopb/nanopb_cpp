#include <vector>
#include <memory>

#include "tests_common.h"
#include "union_message.hpp"

using namespace NanoPb::Converter;


int main() {
    int status = 0;

    const auto messages = LOCAL_UnionContainer::createTestMessages();

    for (auto& original : messages){
        NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

        TEST(NanoPb::encode<UnionContainerConverter>(outputStream, original));

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        LOCAL_UnionContainer decoded;

        TEST(NanoPb::decode<UnionContainerConverter>(inputStream, decoded));

        TEST(original == decoded);
    }

    return status;
}
