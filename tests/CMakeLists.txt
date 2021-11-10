
option(NANOPB_CPP_ENABLE_TESTS_SANITIZE "Enable -fsanitize=address in tests" ON)


option(NANOPB_ROOT "nanopb root dir")

if (NOT NANOPB_ROOT)
    message(FATAL_ERROR "NANOPB_ROOT should be defined")
endif()

set(CMAKE_MODULE_PATH ${NANOPB_ROOT}/extra)
find_package(Nanopb REQUIRED)

nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS
        messages.proto
        )

add_library(lib_test_nanopb STATIC
        ${PROJECT_SOURCE_DIR}/nanopb_cpp.cpp
        helpers.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}
        )

target_include_directories(lib_test_nanopb PUBLIC
        ${JHLIB_PUBLIC_INCLUDE_DIRECTORIES}
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${NANOPB_ROOT}
        )

macro(_add_test TEST_NAME TEST_SOURCES)
    add_executable(${TEST_NAME} ${TEST_SOURCES})

    if (NANOPB_CPP_ENABLE_TESTS_SANITIZE)
        target_link_libraries(${TEST_NAME}  -fsanitize=address)
    endif()

    target_link_libraries(${TEST_NAME} lib_test_nanopb)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()


_add_test(test_map_string_string test/test_map_string_string.cpp)
_add_test(test_map test/test_map_uint32_string.cpp)
_add_test(test_string test/test_string.cpp)

