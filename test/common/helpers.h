#ifndef NANOPB_CPP_HELPERS_H
#define NANOPB_CPP_HELPERS_H

#include <string>
#include <memory>

#include "pb.h"

#ifdef NDEBUG
#error Tests should be compiled in debug mode
#endif

#define STRING_BUFFER_STREAM_MAX_SIZE 65535

#endif //NANOPB_CPP_HELPERS_H
