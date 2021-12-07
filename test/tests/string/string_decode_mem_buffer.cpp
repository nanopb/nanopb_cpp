#include "tests_common.h"
#include "string_common.hpp"

int main() {
    int status = 0;

    const TestMessage original(
            {"My super string"}
    );

    NanoPb::StringOutputStream outputStream;

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto buffer = outputStream.release();

    TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(buffer->data(), buffer->size(), decoded));

    TEST(original == decoded);
    return status;
}
