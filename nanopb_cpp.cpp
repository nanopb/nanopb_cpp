#include "nanopb_cpp.h"

#include "pb_encode.h"
#include "pb_decode.h"

bool NanoPb::Converter::StringConverter::encode(pb_ostream_t *stream, const pb_field_t *field, const std::string *arg) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING);
    if (!pb_encode_tag_for_field(stream, field))
        return false;
    return pb_encode_string(stream, (const pb_byte_t *) arg->c_str(), arg->size());
}

bool NanoPb::Converter::StringConverter::decode(pb_istream_t *stream, const pb_field_t *field, std::string *arg) {
    NANOPB_CPP_ASSERT(PB_LTYPE(field->type) == PB_LTYPE_STRING);
    size_t len = stream->bytes_left;
    arg->resize(len);
    if (!pb_read(stream, (uint8_t *) arg->data(), len)) {
        return false;
    }
    return true;
}

