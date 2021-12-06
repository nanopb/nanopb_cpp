include(nanopb_cpp_tests)

nanopb_cpp_add_test(inner_callback
        SRC inner_callback.cpp
        PROTO
            inner_callback.proto
            ../../common/inner_message.proto
        )