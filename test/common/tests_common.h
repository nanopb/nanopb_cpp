#ifndef NANOPB_CPP_TESTS_COMMON_H
#define NANOPB_CPP_TESTS_COMMON_H

#include <string>
#include <memory>
#include <cstdio>

#include "nanopb_cpp.h"

#ifdef NDEBUG
#error Tests should be compiled in debug mode
#endif

#define STRING_BUFFER_STREAM_MAX_SIZE 65535

#define COMMENT(x) printf("\n----" x "----\n");
#define TEST(x) {}\
    if (!(x)) { \
        fprintf(stderr, "\033[31;1mFAILED:\033[22;39m %s:%d %s\n", __FILE__, __LINE__, #x); \
        status = 1; \
    } else { \
        printf("\033[32;1mOK:\033[22;39m %s\n", #x); \
    }


#endif //NANOPB_CPP_TESTS_COMMON_H
