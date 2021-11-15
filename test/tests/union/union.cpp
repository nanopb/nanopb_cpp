#include <vector>
#include <string>
#include <memory>

#include "tests_common.h"
#include "union_message.hpp"

using namespace NanoPb::Converter;



int testMessage(const LOCAL_UnionMessage& original){
    int status = 0;


    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<UnionMessageConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionMessage decoded;

    TEST(NanoPb::decode<UnionMessageConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}


int main() {
    int status = 0;

    COMMENT("LOCAL_InnerMessageOne");

    auto msg1 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageOne>(
                    new LOCAL_InnerMessageOne(111)
            ));
    status |= testMessage(msg1);

    COMMENT("LOCAL_InnerMessageTwo");
    auto msg2 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageTwo>(
            new LOCAL_InnerMessageTwo("Message number two")
    ));
    status |= testMessage(msg2);

    COMMENT("LOCAL_InnerMessageThree");
    auto msg3 = LOCAL_UnionMessage(std::unique_ptr<LOCAL_InnerMessageThree>(
            new LOCAL_InnerMessageThree({0, 1, 2, 3, UINT32_MAX})
    ));
    status |= testMessage(msg3);

    return status;
}
