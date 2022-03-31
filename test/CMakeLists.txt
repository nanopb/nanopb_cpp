
option(NANOPB_CPP_ENABLE_TESTS_SANITIZE "Enable -fsanitize=address in tests" ON)

list(APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake
        )

find_package(Nanopb REQUIRED)

#file(GLOB TEST_DIRS
#        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*
#        )

set (CMAKE_CXX_STANDARD 11)

add_definitions(-Werror -Wall)

add_subdirectory(tests/enum)
add_subdirectory(tests/inner_callback)
add_subdirectory(tests/map)
add_subdirectory(tests/array)
add_subdirectory(tests/scalar)
add_subdirectory(tests/string)
add_subdirectory(tests/bytes)
add_subdirectory(tests/union)