#include "tests_common.h"
#include "test_string.pb.h"

int main() {
    std::string originalString = "My super string";
    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::encoder(&originalString)
        };

        NANOPB_CPP_ASSERT(pb_encode(&outputStream, &StringContainer_msg, &msg));
    }
    {
        std::string decodedString;
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::decoder(&decodedString)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        NANOPB_CPP_ASSERT(pb_decode(&inputStream, &StringContainer_msg, &msg));

        NANOPB_CPP_ASSERT(originalString == decodedString);
    }

    return 0;
}
