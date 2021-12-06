#ifndef NANOPB_CPP_UNIT_TEST_H
#define NANOPB_CPP_UNIT_TEST_H

#include <cstdio>

#define COMMENT(fmt, ...) printf("\n----" fmt "----\n", ##__VA_ARGS__);
#define TEST(x) {}\
    if (!(x)) { \
        printf("\033[31;1mFAILED:\033[22;39m %s:%d %s\n", __FILE__, __LINE__, #x); \
        status = 1; \
    } else { \
        printf("\033[32;1mOK:\033[22;39m %s\n", #x); \
    }

#endif //NANOPB_CPP_UNIT_TEST_H

#define CONCAT(a,b) a ## b
#define CONCAT3(a,b,c) a ## b ## c
#define CONCAT4(a,b,c,d) a ## b ## c ## d
#define CONCAT5(a,b,c,d,e) a ## b ## c ## d ## e
