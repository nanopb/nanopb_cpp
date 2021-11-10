#ifndef NANOPB_CPP_HELPERS_H
#define NANOPB_CPP_HELPERS_H

#include <string>
#include <memory>

#include "pb.h"
#include "messages.pb.h"

#ifdef NDEBUG
#error Tests should be compiled in debug mode
#endif

class OutputStream {
public:
    OutputStream();
    pb_ostream_t* getStream();
    const pb_byte_t * getData();
    size_t getDataSize();
private:
    std::unique_ptr<std::string> _strBuffer;
    std::unique_ptr<pb_ostream_t> _ostream;
};

#endif //NANOPB_CPP_HELPERS_H
