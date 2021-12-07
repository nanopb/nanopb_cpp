#include <float.h>

#include "tests_common.h"
#include "nanopb_cpp.h"

using namespace NanoPb::Type;

template <class SCALAR>
bool scalarTest(typename SCALAR::LocalType original){
    NanoPb::StringOutputStream outputStream;

    if (!SCALAR::encode(&outputStream, original))
        return false;

    auto inputStream = NanoPb::StringInputStream(outputStream.release());

    typename SCALAR::LocalType decoded;

    if (!SCALAR::decode(&inputStream, decoded))
        return false;

    return original == decoded;
}

#ifndef PB_WITHOUT_64BIT
#endif

int main() {
    int status = 0;

    TEST(scalarTest<Int32>(INT32_MIN));
    TEST(scalarTest<Int32>(INT32_MAX));

    TEST(scalarTest<SInt32>(INT32_MIN));
    TEST(scalarTest<SInt32>(INT32_MAX));

    TEST(scalarTest<UInt32>(0));
    TEST(scalarTest<UInt32>(UINT32_MAX));

    TEST(scalarTest<Fixed32>(0));
    TEST(scalarTest<Fixed32>(UINT32_MAX));

    TEST(scalarTest<SFixed32>(INT32_MIN));
    TEST(scalarTest<SFixed32>(INT32_MAX));

    TEST(scalarTest<Float>(FLT_MIN));
    TEST(scalarTest<Float>(FLT_MAX));

    TEST(scalarTest<Bool>(true));
    TEST(scalarTest<Bool>(false));

    TEST(scalarTest<String>("My super string"));
    TEST(scalarTest<String>(""));

    TEST(scalarTest<Bytes>("Bytes"));
    TEST(scalarTest<Bytes>(""));

#ifndef PB_WITHOUT_64BIT

    TEST(scalarTest<Int64>(INT64_MIN));
    TEST(scalarTest<Int64>(INT64_MAX));

    TEST(scalarTest<SInt64>(INT64_MIN));
    TEST(scalarTest<SInt64>(INT64_MAX));

    TEST(scalarTest<UInt64>(0));
    TEST(scalarTest<UInt64>(UINT64_MAX));

    TEST(scalarTest<Fixed64>(0));
    TEST(scalarTest<Fixed64>(UINT64_MAX));

    TEST(scalarTest<SFixed64>(INT64_MIN));
    TEST(scalarTest<SFixed64>(INT64_MAX));

    TEST(scalarTest<Double>(DBL_MIN));
    TEST(scalarTest<Double>(DBL_MAX));

#endif

    return status;
}
