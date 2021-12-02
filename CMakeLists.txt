cmake_minimum_required(VERSION 3.16)
project(nanopb_cpp)

set(NANOPB_VERSION 0.4.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CPM)

# Custom nanopb location
if (NANOPB_ROOT)
    set(CPM_lib_nanopb_SOURCE ${NANOPB_ROOT})
endif()

set(NANOPB_CPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

CPMAddPackage(NAME lib_nanopb GITHUB_REPOSITORY nanopb/nanopb GIT_TAG ${NANOPB_VERSION} DOWNLOAD_ONLY YES)
set(NANOPB_ROOT ${lib_nanopb_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 14)

enable_testing()

add_subdirectory(test)
