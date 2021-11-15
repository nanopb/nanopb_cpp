include(nanopb_cpp_tests)

nanopb_cpp_add_test(string
        SRC string.cpp
        PROTO string.proto
        )

nanopb_cpp_add_test(string_mem_buffer
        SRC string_decode_mem_buffer.cpp
        PROTO string.proto
        )