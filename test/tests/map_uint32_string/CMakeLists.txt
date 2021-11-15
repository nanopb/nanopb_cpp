include(nanopb_cpp_tests)

nanopb_cpp_add_test(map_uint32_string
        SRC map_uint32_string.cpp
        PROTO map_uint32_string.proto
        )