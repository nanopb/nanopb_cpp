set(TEST_COMMON_DIR ${CMAKE_CURRENT_LIST_DIR}/../common)

function(nanopb_cpp_add_test TEST)
    set(EXECUTABLE "test_${TEST}")
    set(TEST_FOLDER  ${CMAKE_CURRENT_SOURCE_DIR})

    if(NOT EXISTS "${TEST_FOLDER}")
        message(FATAL_ERROR "Test folder does not exists: ${TEST_FOLDER}")
    endif()

    file(GLOB_RECURSE TEST_SOURCES
            ${TEST_FOLDER}/*.cpp
            )
    if (NOT TEST_SOURCES)
        message(FATAL_ERROR "No sources found in ${TEST_FOLDER}")
    endif()

    file(GLOB_RECURSE TEST_PROTOS
            ${TEST_FOLDER}/*.proto
            )

    if (NOT TEST_PROTOS)
        message(FATAL_ERROR "No .proto found in ${TEST_FOLDER}")
    endif()

    set(NANOPB_OPTIONS --error-on-unmatched)

    nanopb_generate_cpp(
            PROTO_SRCS
            PROTO_HDRS
            ${TEST_PROTOS}
            ${TEST_COMMON_DIR}/inner_message.proto
    )

    add_executable(${EXECUTABLE}
            ${TEST_SOURCES}
            ${PROTO_SRCS}
            ${PROTO_HDRS}
            ${PROJECT_SOURCE_DIR}/nanopb_cpp.cpp
            )

    target_include_directories(${EXECUTABLE} PRIVATE
            ${TEST_FOLDER}
            ${PROJECT_SOURCE_DIR}
            ${TEST_COMMON_DIR}
            ${NANOPB_ROOT}
            ${CMAKE_CURRENT_BINARY_DIR}
            )

    if (NANOPB_CPP_ENABLE_TESTS_SANITIZE)
        target_link_libraries(${EXECUTABLE}  -fsanitize=address)
    endif()

    add_test(NAME ${TEST} COMMAND ${EXECUTABLE})
endfunction()