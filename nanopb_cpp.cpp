#include "nanopb_cpp.h"

#include "pb_encode.h"
#include "pb_decode.h"

NanoPb::StringOutputStream::StringOutputStream(size_t maxSize) : _buffer(std::make_unique<NanoPb::BufferType>()) {
    callback = +[](pb_ostream_t *stream, const pb_byte_t *buf, size_t count) -> bool {
        auto strBuffer = static_cast<NanoPb::BufferPtr *>(stream->state);
        NANOPB_CPP_ASSERT(strBuffer);
        if (!strBuffer)
            return false;
        (*strBuffer)->append((char*)buf, count);
        return true;
    };
    state = &_buffer;
    max_size = maxSize;
    bytes_written = 0;
#ifndef PB_NO_ERRMSG
    errmsg = NULL;
#endif
}

NanoPb::BufferPtr NanoPb::StringOutputStream::release() {
    return std::move(_buffer);
}

bool NanoPb::Converter::StringConverter::_encode(pb_ostream_t *stream, const pb_field_t *field, const LocalType *arg) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_string(stream, (const pb_byte_t *) arg->c_str(), arg->size());
}

bool NanoPb::Converter::StringConverter::_decode(pb_istream_t *stream, const pb_field_t *field, LocalType *arg) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING);
    size_t len = stream->bytes_left;
    arg->resize(len);
    if (!pb_read(stream, (uint8_t *) arg->data(), len)) {
        return false;
    }
    return true;
}

NanoPb::StringInputStream::StringInputStream(BufferPtr &&buffer) : _buffer(std::move(buffer)), _position(0) {
    callback = +[](pb_istream_t *stream, pb_byte_t *buf, size_t count) -> bool {
        auto self = static_cast<NanoPb::StringInputStream *>(stream->state);

        NANOPB_CPP_ASSERT(self->_position < self->_buffer->size());
        if (self->_position >= self->_buffer->size())
            return false;
        if (buf != NULL)
        {
            for (size_t i = 0; i < count; i++)
                buf[i] = self->_buffer->at(self->_position + i);
        }

        self->_position += count;

        return true;
    };
    state = this;
    bytes_left = _buffer->size();
#ifndef PB_NO_ERRMSG
    errmsg = NULL;
#endif
}
