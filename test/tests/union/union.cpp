#include <vector>
#include <string>
#include <memory>

#include "tests_common.h"
#include "union_message.hpp"

using namespace NanoPb::Converter;



int testMessage(const LOCAL_UnionContainer& original){
    int status = 0;


    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    TEST(NanoPb::encode<UnionContainerConverter>(outputStream, original));

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    LOCAL_UnionContainer decoded;

    TEST(NanoPb::decode<UnionContainerConverter>(inputStream, decoded));

    TEST(original == decoded);

    return status;
}


int main() {
    int status = 0;

    COMMENT("LOCAL_UnionInnerOne");

    auto msg1 = LOCAL_UnionContainer(std::unique_ptr<LOCAL_UnionInnerOne>(
                    new LOCAL_UnionInnerOne(111)
            ));
    status |= testMessage(msg1);

    COMMENT("LOCAL_UnionInnerTwo");
    auto msg2 = LOCAL_UnionContainer(std::unique_ptr<LOCAL_UnionInnerTwo>(
            new LOCAL_UnionInnerTwo("Message number two")
    ));
    status |= testMessage(msg2);

    COMMENT("LOCAL_UnionInnerThree");
    auto msg3 = LOCAL_UnionContainer(std::unique_ptr<LOCAL_UnionInnerThree>(
            new LOCAL_UnionInnerThree({0, 1, 2, 3, UINT32_MAX})
    ));
    status |= testMessage(msg3);

    return status;
}
