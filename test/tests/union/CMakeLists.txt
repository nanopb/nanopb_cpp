include(nanopb_cpp_tests)

nanopb_cpp_add_test(union
        SRC union.cpp
        PROTO ../../common/union_inner_messages.proto
        PROTO ../../common/union_container_no_union.proto
        )