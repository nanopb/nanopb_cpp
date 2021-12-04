#include "nanopb_cpp.h"

#include "pb_encode.h"
#include "pb_decode.h"

#ifdef PB_WITHOUT_64BIT
#define pb_int64_t int32_t
#define pb_uint64_t uint32_t
#else
#define pb_int64_t int64_t
#define pb_uint64_t uint64_t
#endif

/****************************************************************************************************************/

NanoPb::StringOutputStream::StringOutputStream(size_t maxSize) : _buffer(new NanoPb::BufferType()) {
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

/****************************************************************************************************************/

NanoPb::BufferPtr NanoPb::StringOutputStream::release() {
    return std::move(_buffer);
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

/****************************************************************************************************************/

const pb_msgdesc_t *NanoPb::decodeUnionMessageType(pb_istream_t &stream, const pb_msgdesc_t *unionContainer) {
    pb_wire_type_t wire_type;
    uint32_t tag;
    bool eof;

    while (pb_decode_tag(&stream, &wire_type, &tag, &eof))
    {
        if (wire_type == PB_WT_STRING)
        {
            pb_field_iter_t iter;
            if (pb_field_iter_begin(&iter, unionContainer, NULL) &&
                pb_field_iter_find(&iter, tag))
            {
                /* Found our field. */
                return iter.submsg_desc;
            }
        }

        /* Wasn't our field.. */
        pb_skip_field(&stream, wire_type);
    }

    return NULL;
}

/****************************************************************************************************************/

bool NanoPb::Converter::StringCallbackConverter::encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING || PB_LTYPE(field->type) == PB_LTYPE_BYTES);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_string(stream, (const pb_byte_t *) local.c_str(), local.size());
}

bool NanoPb::Converter::StringCallbackConverter::decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING || PB_LTYPE(field->type) == PB_LTYPE_BYTES);
    size_t len = stream->bytes_left;
    local.resize(len);
    if (!pb_read(stream, (uint8_t *) local.data(), len)) {
        return false;
    }
    return true;
}

/****************************************************************************************************************/

bool NanoPb::ScalarType::Int32::rawEncode(pb_ostream_t *stream, const int32_t &value) {
    pb_int64_t v = value;
    return pb_encode_svarint(stream, v);
}

bool NanoPb::ScalarType::Int32::rawDecode(pb_istream_t *stream, int32_t &value) {
    pb_int64_t v;
    if (!pb_decode_svarint(stream, &v))
        return false;
    value = v;
    return true;
}

/******************************************/

bool NanoPb::ScalarType::SInt32::rawEncode(pb_ostream_t *stream, const int32_t &value) {
    pb_int64_t v = value;
    return pb_encode_svarint(stream, v);
}

bool NanoPb::ScalarType::SInt32::rawDecode(pb_istream_t *stream, int32_t &value) {
    pb_int64_t v;
    if (!pb_decode_svarint(stream, &v))
        return false;
    value = v;
    return true;
}

/******************************************/

bool NanoPb::ScalarType::UInt32::rawEncode(pb_ostream_t *stream, const uint32_t &value) {
    return pb_encode_varint(stream, value);
}

bool NanoPb::ScalarType::UInt32::rawDecode(pb_istream_t *stream, uint32_t &value) {
    return pb_decode_varint32(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::Fixed32::rawEncode(pb_ostream_t *stream, const uint32_t &value) {
    return pb_encode_fixed32(stream, &value);
}

bool NanoPb::ScalarType::Fixed32::rawDecode(pb_istream_t *stream, uint32_t &value) {
    return pb_decode_fixed32(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::SFixed32::rawEncode(pb_ostream_t *stream, const int32_t &value) {
    return pb_encode_fixed32(stream, &value);
}

bool NanoPb::ScalarType::SFixed32::rawDecode(pb_istream_t *stream, int32_t &value) {
    return pb_decode_fixed32(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::Float::rawEncode(pb_ostream_t *stream, const float &value) {
    return pb_encode_fixed32(stream, &value);
}

bool NanoPb::ScalarType::Float::rawDecode(pb_istream_t *stream, float &value) {
    return pb_decode_fixed32(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::Bool::rawEncode(pb_ostream_t *stream, const bool &value) {
    uint32_t v = value;
    return pb_encode_varint(stream, v);
}

bool NanoPb::ScalarType::Bool::rawDecode(pb_istream_t *stream, bool &value) {
    uint32_t v;
    if (!pb_decode_varint32(stream, &v))
        return false;
    value = v;
    return true;
}

/******************************************/

bool NanoPb::ScalarType::String::rawEncode(pb_ostream_t *stream, const std::string &value) {
    return pb_write(stream, (const pb_byte_t *) value.data(), value.size());
}

bool NanoPb::ScalarType::String::rawDecode(pb_istream_t *stream, std::string &value) {
    size_t len = stream->bytes_left;
    value.resize(len);
    return pb_read(stream, (pb_byte_t *) value.data(), len);
}

/******************************************/

bool NanoPb::ScalarType::Bytes::rawEncode(pb_ostream_t *stream, const std::string &value) {
    return String::rawEncode(stream, value);
}

bool NanoPb::ScalarType::Bytes::rawDecode(pb_istream_t *stream, std::string &value) {
    return String::rawDecode(stream, value);
}

/******************************************/

#ifndef PB_WITHOUT_64BIT

bool NanoPb::ScalarType::Int64::rawEncode(pb_ostream_t *stream, const int64_t &value) {
    return pb_encode_svarint(stream, value);
}

bool NanoPb::ScalarType::Int64::rawDecode(pb_istream_t *stream, int64_t &value) {
    pb_int64_t v;
    if (!pb_decode_svarint(stream, &v))
        return false;
    value = v;
    return true;
}

/******************************************/

bool NanoPb::ScalarType::SInt64::rawEncode(pb_ostream_t *stream, const int64_t &value) {
    pb_int64_t v = value;
    return pb_encode_svarint(stream, v);
}

bool NanoPb::ScalarType::SInt64::rawDecode(pb_istream_t *stream, int64_t &value) {
    pb_int64_t v;
    if (!pb_decode_svarint(stream, &v))
        return false;
    value = v;
    return true;
}
/******************************************/


bool NanoPb::ScalarType::UInt64::rawEncode(pb_ostream_t *stream, const uint64_t &value) {
    return pb_encode_varint(stream, value);
}

bool NanoPb::ScalarType::UInt64::rawDecode(pb_istream_t *stream, uint64_t &value) {
    pb_uint64_t v;
    if (!pb_decode_varint(stream, &v))
        return false;
    value = v;
    return true;
}

/******************************************/

bool NanoPb::ScalarType::Fixed64::rawEncode(pb_ostream_t *stream, const uint64_t &value) {
    return pb_encode_fixed64(stream, &value);
}

bool NanoPb::ScalarType::Fixed64::rawDecode(pb_istream_t *stream, uint64_t &value) {
    return pb_decode_fixed64(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::SFixed64::rawEncode(pb_ostream_t *stream, const int64_t &value) {
    return pb_encode_fixed64(stream, &value);
}

bool NanoPb::ScalarType::SFixed64::rawDecode(pb_istream_t *stream, int64_t &value) {
    return pb_decode_fixed64(stream, &value);
}

/******************************************/

bool NanoPb::ScalarType::Double::rawEncode(pb_ostream_t *stream, const double &value) {
    return pb_encode_fixed64(stream, &value);
}

bool NanoPb::ScalarType::Double::rawDecode(pb_istream_t *stream, double &value) {
    return pb_decode_fixed64(stream, &value);
}
#endif

/****************************************************************************************************************/
