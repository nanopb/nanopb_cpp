

find_package(Nanopb REQUIRED)

set(NANOPB_OPTIONS --error-on-unmatched)

nanopb_generate_cpp(
        PROTO_SRCS
        PROTO_HDRS

        # Specify full path to proto file.
        # In other case .option file will be not parsed.
        # TODO: Is it bug or feature of the nanopb generator?
        ${CMAKE_CURRENT_SOURCE_DIR}/complex.proto
)

add_executable(complex
        main.cpp
        ${PROTO_SRCS} ${PROTO_HDRS}
        )
target_include_directories(complex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(complex nanopb_cpp)
