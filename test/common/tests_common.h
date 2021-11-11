#ifndef NANOPB_CPP_TESTS_COMMON_H
#define NANOPB_CPP_TESTS_COMMON_H

#include <string>
#include <memory>

#include "nanopb_cpp.h"

#ifdef NDEBUG
#error Tests should be compiled in debug mode
#endif

#define STRING_BUFFER_STREAM_MAX_SIZE 65535

#endif //NANOPB_CPP_TESTS_COMMON_H
