#include <vector>
#include <list>

#include "tests_common.h"
#include "repeated_unsigned.pb.h"

using namespace NanoPb::Converter;


template <class CONTAINER>
int testRepeated(typename CONTAINER::value_type minValue, typename CONTAINER::value_type maxValue){

    int status = 0;

    const CONTAINER original = {minValue, 0, 1 , 2, 3, 4, 5, maxValue};

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        RepeatedUnsignedContainer msg = {
                .values = RepeatedUnsignedConverter<CONTAINER>::encoder(&original)
        };

        TEST(pb_encode(&outputStream, &RepeatedUnsignedContainer_msg, &msg));
    }

    {
        CONTAINER decoded;

        RepeatedUnsignedContainer msg = {
                .values = RepeatedUnsignedConverter<CONTAINER>::decoder(&decoded)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &RepeatedUnsignedContainer_msg, &msg));

        TEST(original == decoded);
    }

    return status;
}

int main() {
    int status = 0;

    status |= testRepeated<std::vector<uint64_t>>(0, UINT64_MAX);
    status |= testRepeated<std::list<uint64_t>>(0, UINT64_MAX);

    status |= testRepeated<std::vector<uint32_t>>(0, UINT32_MAX);
    status |= testRepeated<std::list<uint32_t>>(0, UINT32_MAX);

    return status;
}
