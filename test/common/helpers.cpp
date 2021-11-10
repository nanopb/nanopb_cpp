#include "helpers.h"
#include "pb_encode.h"

#define STRING_BUFFER_STREAM_MAX_SIZE 65535

OutputStream::OutputStream() :
        _strBuffer(std::make_unique<std::string>()),
        _ostream(std::make_unique<pb_ostream_t>())
{
    *_ostream = {
            .callback = +[](pb_ostream_t *stream, const pb_byte_t *buf, size_t count) -> bool {
                std::string* str = static_cast<std::string *>(stream->state);;
                str->append((char*)buf, count);
                return true;
                },
            .state = _strBuffer.get(),
            .max_size = STRING_BUFFER_STREAM_MAX_SIZE
    };
}

pb_ostream_t* OutputStream::getStream() {
    return _ostream.get();
}

const pb_byte_t * OutputStream::getData() {
    return (pb_byte_t *)(_strBuffer->data());
}

size_t OutputStream::getDataSize() {
    return _strBuffer->length();
}
