include(nanopb_cpp_tests)

nanopb_cpp_add_test(array
        SRC array.cpp
        PROTO
            array.proto
            ../../common/simple_enum.proto
            ../../common/inner_message.proto

        )