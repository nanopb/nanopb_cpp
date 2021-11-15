include(nanopb_cpp_tests)

nanopb_cpp_add_test(map_string_message
        SRC map_string_message.cpp
        PROTO
            map_string_message.proto
            ../../common/inner_message.proto
        )