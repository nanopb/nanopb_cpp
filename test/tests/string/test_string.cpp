#include "tests_common.h"
#include "test_string.pb.h"

int main() {
    int status = 0;

    std::string originalString = "My super string";
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::encoder(&originalString)
        };

        TEST(pb_encode(&outputStream, &StringContainer_msg, &msg));
    }
    {
        std::string decodedString;
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::decoder(&decodedString)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &StringContainer_msg, &msg));

        TEST(originalString == decodedString);
    }

    return status;
}
