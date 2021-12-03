cmake_minimum_required(VERSION 3.16)
project(nanopb_cpp)

set(CMAKE_CXX_STANDARD 14)

#
# Options
#
set(NANOPB_VERSION 0.4.5 CACHE STRING "nanopb version")
option(BUILD_SHARED "Build shared instead of static" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(PB_WITHOUT_64BIT "Build nanopb without 64-bit support" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CPM)

# Custom nanopb location
if (NANOPB_ROOT)
    set(CPM_lib_nanopb_SOURCE ${NANOPB_ROOT})
endif()

CPMAddPackage(NAME lib_nanopb GITHUB_REPOSITORY nanopb/nanopb GIT_TAG ${NANOPB_VERSION} DOWNLOAD_ONLY YES)
set(NANOPB_ROOT ${lib_nanopb_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH CMAKE_MODULE_PATH ${NANOPB_ROOT}/extra)

if (PB_WITHOUT_64BIT)
    add_definitions(-DPB_WITHOUT_64BIT)
endif()

if (BUILD_SHARED)
    add_library(nanopb_cpp SHARED nanopb_cpp.cpp)
else()
    add_library(nanopb_cpp STATIC nanopb_cpp.cpp)
endif()

target_include_directories(nanopb_cpp PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${lib_nanopb_SOURCE_DIR}
        )

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
