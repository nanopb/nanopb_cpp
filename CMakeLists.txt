cmake_minimum_required(VERSION 3.16)
project(nanopb_cpp)

set(CMAKE_CXX_STANDARD 11)

#
# Options
#
set(NANOPB_VERSION master CACHE STRING "nanopb version")
option(BUILD_SHARED "Build shared instead of static" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(PB_WITHOUT_64BIT "Build nanopb without 64-bit support" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CPM)

CPMAddPackage(NAME lib_nanopb GITHUB_REPOSITORY nanopb/nanopb GIT_TAG ${NANOPB_VERSION} DOWNLOAD_ONLY YES)

list(APPEND CMAKE_MODULE_PATH CMAKE_MODULE_PATH ${lib_nanopb_SOURCE_DIR}/extra)

if (PB_WITHOUT_64BIT)
    add_definitions(-DPB_WITHOUT_64BIT)
endif()

set(SOURCES
        nanopb_cpp.cpp
        )

if (BUILD_SHARED)
    add_library(nanopb_cpp SHARED ${SOURCES})
else()
    add_library(nanopb_cpp STATIC ${SOURCES})
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
