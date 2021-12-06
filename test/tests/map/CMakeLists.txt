include(nanopb_cpp_tests)

nanopb_cpp_add_test(map
        SRC map.cpp
        PROTO
            map.proto
            ../../common/simple_enum.proto
            ../../common/inner_message.proto

        )