include(nanopb_cpp_tests)

nanopb_cpp_add_test(array_message
        SRC array_message.cpp
        PROTO array_message.proto
        ../../common/inner_message.proto
        )