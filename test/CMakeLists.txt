
option(NANOPB_CPP_ENABLE_TESTS_SANITIZE "Enable -fsanitize=address in tests" ON)


option(NANOPB_ROOT "nanopb root dir")

if (NOT NANOPB_ROOT)
    message(FATAL_ERROR "NANOPB_ROOT should be defined")
endif()


list(APPEND CMAKE_MODULE_PATH
        CMAKE_MODULE_PATH ${NANOPB_ROOT}/extra
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        )

find_package(Nanopb REQUIRED)

#file(GLOB TEST_DIRS
#        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*
#        )

set (CMAKE_CXX_STANDARD 11)

add_definitions(-Wall)

add_subdirectory(tests/enum)
add_subdirectory(tests/map_string_string)
add_subdirectory(tests/map_uint32_string)
add_subdirectory(tests/repeated_signed)
add_subdirectory(tests/repeated_unsigned)
add_subdirectory(tests/string)