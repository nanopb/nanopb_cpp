#include "tests_common.h"
#include "string_common.hpp"

int main() {
    int status = 0;

    const LOCAL_TestMessage original(
            {"My super string"}
    );

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<TestMessageConverter>(outputStream, original));

    auto buffer = outputStream.release();

    LOCAL_TestMessage decoded;

    TEST(NanoPb::decode<TestMessageConverter>(buffer->data(), buffer->size(), decoded));

    TEST(original == decoded);
    return status;
}
