#include "tests_common.h"
#include "string_common.hpp"

int main() {
    int status = 0;

    const TestMessage original(
            {"My super string"}
            );

    NanoPb::StringOutputStream outputStream;

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(inputStream, decoded));

    TEST(original == decoded);
    return status;
}
