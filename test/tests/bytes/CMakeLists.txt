include(nanopb_cpp_tests)

nanopb_cpp_add_test(bytes
        SRC bytes.cpp
        PROTO bytes.proto
        )