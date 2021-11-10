#include <iostream>

#include "nanopb_cpp.h"
#include "test_string.pb.h"
#include "helpers.h"

int main() {
    std::string originalString = "My super string";
    OutputStream outputStream;

    {
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::encoder(&originalString)
        };

        NANOPB_CPP_ASSERT(pb_encode(outputStream.getStream(), &StringContainer_msg, &msg));
    }
    {
        std::string decodedString;
        StringContainer msg = {
                .str = NanoPb::Converter::StringConverter::decoder(&decodedString)
        };

        pb_istream_t stream = pb_istream_from_buffer(outputStream.getData(), outputStream.getDataSize());

        NANOPB_CPP_ASSERT(pb_decode(&stream, &StringContainer_msg, &msg));

        NANOPB_CPP_ASSERT(originalString == decodedString);
    }

    return 0;
}
