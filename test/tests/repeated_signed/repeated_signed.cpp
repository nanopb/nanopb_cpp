#include <vector>
#include <list>

#include "tests_common.h"
#include "repeated_signed.pb.h"

using namespace NanoPb::Converter;


template <class CONTAINER>
int testRepeated(const typename CONTAINER::value_type minValue, const typename CONTAINER::value_type maxValue){
    int status = 0;

    CONTAINER original = {minValue, 0, 1 , 2, 3, 4, 5, maxValue};

    NanoPb::StringOutputStream outputStream(STRING_BUFFER_STREAM_MAX_SIZE);

    {
        RepeatedSignedContainer msg = {
                .values = RepeatedSignedConverter<CONTAINER>::encoder(&original)
        };

        TEST(pb_encode(&outputStream, &RepeatedSignedContainer_msg, &msg));
    }

    {
        CONTAINER decoded;

        RepeatedSignedContainer msg = {
                .values = RepeatedSignedConverter<CONTAINER>::decoder(&decoded)
        };

        auto inputStream = NanoPb::StringInputStream(outputStream.release());

        TEST(pb_decode(&inputStream, &RepeatedSignedContainer_msg, &msg));

        TEST(original == decoded);
    }

    return status;
}

int main() {
    int status = 0;

    status |= testRepeated<std::vector<int64_t>>(INT64_MIN, INT64_MAX);
    status |= testRepeated<std::list<int64_t>>(INT64_MIN, INT64_MAX);

    status |= testRepeated<std::vector<int32_t>>(INT32_MIN, INT32_MAX);
    status |= testRepeated<std::list<int32_t>>(INT32_MIN, INT32_MAX);

    return status;
}
