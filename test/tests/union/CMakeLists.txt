include(nanopb_cpp_tests)

nanopb_cpp_add_test(union
        SRC union.cpp
        PROTO inner_messages.proto
        PROTO container.proto
        )