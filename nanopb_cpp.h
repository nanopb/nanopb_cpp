#ifndef NANOPB_CPP_NANOPB_CPP_H
#define NANOPB_CPP_NANOPB_CPP_H

#include <string>
#include <memory>

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "pb_common.h"

#ifndef NANOPB_CPP_ASSERT
#ifndef NDEBUG
#include <cassert>
#define NANOPB_CPP_ASSERT(expr) assert(expr)
#else
#define _nanopb_cpp_unused(x) ((void)(x))
#define NANOPB_CPP_ASSERT(expr) _nanopb_cpp_unused(expr)
#endif
#endif

namespace NanoPb {

    using BufferType = std::string;
    using BufferPtr = std::unique_ptr<BufferType>;

    /**
     * StringOutputStream
     */
    class StringOutputStream : public pb_ostream_t {
    public:
        // FIXME: Is maxSize really needed? We can dynamically increase max size on each write.
        //  Need to ask PetteriAimonen
        //  Update README.md after fix
        StringOutputStream(size_t maxSize);
        BufferPtr release();
    private:
        BufferPtr _buffer;
    };

    /**
     * StringInputStream
     */
    class StringInputStream : public pb_istream_t {
    public:
        StringInputStream(BufferPtr&& buffer);
    private:
        BufferPtr _buffer;
        size_t _position;
    };

    /**
     * Encode message
     */
    template<class MESSAGE_CONVERTER>
    bool encode(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& v){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using LocalType = typename MESSAGE_CONVERTER::LocalType;

        const LocalType& local = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(local);

        return pb_encode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeSubMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& v){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using LocalType = typename MESSAGE_CONVERTER::LocalType;

        const LocalType& local = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(local);

        return pb_encode_submessage(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode union message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeUnionMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& v, const pb_msgdesc_t* unionContainer){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using LocalType = typename MESSAGE_CONVERTER::LocalType;

        pb_field_iter_t iter;

        const LocalType& local = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(local);

        if (!pb_field_iter_begin(&iter, unionContainer, &proto))
            return false;
        do
        {
            if (iter.submsg_desc == MESSAGE_CONVERTER::getMsgType()){
                /* This is our field, encode the message using it. */
                if (!pb_encode_tag_for_field(&stream, &iter))
                    return false;
                return pb_encode_submessage(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
            }
        } while (pb_field_iter_next(&iter));

        /* Didn't find the field for messagetype */
        return false;
    }

    /**
     * Decode message from stream
     */
    template<class MESSAGE_CONVERTER>
    bool decode(pb_istream_t &stream, typename MESSAGE_CONVERTER::LocalType& v){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using LocalType = typename MESSAGE_CONVERTER::LocalType;

        LocalType& local = v;
        ProtoType proto = MESSAGE_CONVERTER::decoderInit(local);

        if (!pb_decode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto))
            return false;
        if (!MESSAGE_CONVERTER::decoderApply(proto, local))
            return false;
        return true;
    }

    /**
     * Decode from buffer in memory
     */
    template<class MESSAGE_CONVERTER>
    bool decode(const void* data, const pb_size_t dataSize, typename MESSAGE_CONVERTER::LocalType& v){
        pb_istream_t stream = pb_istream_from_buffer((pb_byte_t*)data, dataSize);
        return decode<MESSAGE_CONVERTER>(stream, v);
    }

    /**
     * Decode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool decodeSubMessage(pb_istream_t &stream, typename MESSAGE_CONVERTER::LocalType& v){
        pb_istream_t subStream;

        if (!pb_make_string_substream(&stream, &subStream))
            return false;

        if (!decode<MESSAGE_CONVERTER>(subStream, v))
            return false;

        if (!pb_close_string_substream(&stream, &subStream))
            return false;
        return true;
    }

    /**
     * Decode union message type
     */
    const pb_msgdesc_t* decodeUnionMessageType(pb_istream_t &stream, const pb_msgdesc_t* unionContainer);

    /**
     * Basic Scalar types.
     *
     *  Should implement
     *      rawEncode()/rawDecode()
     *
     */
    namespace Type {
        template<class TYPE, class LOCAL_TYPE>
        class AbstractScalarType {
        public:
            using LocalType = LOCAL_TYPE;
        };

        class Int32 : public AbstractScalarType<Int32, int32_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class SInt32 : public AbstractScalarType<SInt32, int32_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class UInt32 : public AbstractScalarType<UInt32, uint32_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class Fixed32 : public AbstractScalarType<Fixed32, uint32_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class SFixed32 : public AbstractScalarType<SFixed32, int32_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class Float : public AbstractScalarType<Float, float>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class Bool : public AbstractScalarType<Bool, bool>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        /**
         * NOTE: rawEncode()/rawDecode() **does NOT** add length for String/Bytes
         */
        class String : public AbstractScalarType<String, std::string>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        /**
         * NOTE: rawEncode()/rawDecode() **does NOT** add length for String/Bytes
         */
        class Bytes : public AbstractScalarType<Bytes, std::string>{ // use std::string as container
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

#ifndef PB_WITHOUT_64BIT
        class Int64 : public AbstractScalarType<Int64, int64_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class SInt64 : public AbstractScalarType<SInt64, int64_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class UInt64 : public AbstractScalarType<UInt64, uint64_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class Fixed64 : public AbstractScalarType<Fixed64, uint64_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class SFixed64 : public AbstractScalarType<SFixed64, int64_t>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };

        class Double : public AbstractScalarType<Double, double>{
        public:
            static bool rawEncode(pb_ostream_t *stream, const LocalType& value);
            static bool rawDecode(pb_istream_t *stream, LocalType& value);
        };
#endif
    }

    namespace Converter {
        /**
         * Enum converter
         *
         * @tparam CONVERTER - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE>
        class EnumConverter {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:  // Should be overwritten in child class

            static ProtoType encode(const LocalType& local);
            static LocalType decode(const ProtoType& proto);
        };


        /**
         * Message converter
         *
         * @tparam CONVERTER - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         * @tparam PROTO_TYPE_MSG - NanoPb msg descriptor
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class MessageConverter {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

        public:  // Should be overwritten in child class

            static ProtoType encoderInit(const LocalType& local);
            static ProtoType decoderInit(LocalType& local);
            static bool decoderApply(const ProtoType& proto, LocalType& local);
        };

        /**
         * Union message converter
         *
         * @tparam CONVERTER - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         * @tparam PROTO_TYPE_MSG - NanoPb msg descriptor
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class UnionMessageConverter : public MessageConverter<CONVERTER, LOCAL_TYPE, PROTO_TYPE, PROTO_TYPE_MSG>{
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

            static pb_callback_t unionDecoderInit(LocalType& local) { return pb_callback_t{ .funcs = { .decode = _unionDecodeCallback }, .arg = (void*)&local }; }

        public:  // Should be overwritten in child class

            static ProtoType encoderInit(const LocalType& local);
            static ProtoType decoderInit(LocalType& local);
            static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
            static bool decoderApply(const ProtoType& proto, LocalType& local);

        private:
            static bool _unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::unionDecodeCallback(stream, field, *(static_cast<LocalType *>(*arg)));
            }
        };

        /**
         * Callback converter
         *
         * @tparam CONVERTER - Derived class
         * @tparam LOCAL_TYPE - Local type
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class CallbackConverter {
        public:
            using LocalType = LOCAL_TYPE;
        public:
            static pb_callback_t encoderCallbackInit(const LocalType& local) { return pb_callback_t{ .funcs = { .encode = _pbEncodeCallback }, .arg = (void*)&local }; }
            static pb_callback_t decoderCallbackInit(LocalType& local) { return pb_callback_t{ .funcs = { .decode = _pbDecodeCallback }, .arg = (void*)&local }; }

        public:  // Should be overwritten in child class

            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);

        private:
            static bool _pbEncodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::encodeCallback(stream, field, *(static_cast<const LocalType *>(*arg)));
            };
            static bool _pbDecodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::decodeCallback(stream, field, *(static_cast<LocalType *>(*arg)));
            };
        };

        /**
         * Basic scalar converter.
         *
         *  Can be used for field with wire types: PB_WT_VARINT, PB_WT_64BIT, PB_WT_32BIT
         *
         * @tparam CONVERTER - Derived class
         * @tparam SCALAR - Scalar with one of listed above wire type.
         */
        template <class CONVERTER, class SCALAR>
        class AbstractScalarConverter : public CallbackConverter<CONVERTER, typename SCALAR::LocalType> {
        public:
            using LocalType = typename SCALAR::LocalType;
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local){
                //FIXME: Add assert on field type
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                return SCALAR::rawEncode(stream, local);
            }
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
                return SCALAR::rawDecode(stream, local);
            }
        };

        /**
         * Set of basic scalar types converters to used in ArrayConverter and other cases
         */
        class Int32Converter : public AbstractScalarConverter<Int32Converter,Type::Int32> {};
        class SInt32Converter : public AbstractScalarConverter<SInt32Converter,Type::SInt32> {};
        class UInt32Converter : public AbstractScalarConverter<UInt32Converter,Type::UInt32> {};
        class Fixed32Converter : public AbstractScalarConverter<Fixed32Converter,Type::Fixed32> {};
        class SFixed32Converter : public AbstractScalarConverter<SFixed32Converter,Type::SFixed32> {};
        class FloatConverter : public AbstractScalarConverter<FloatConverter,Type::Float> {};
        class BoolConverter : public AbstractScalarConverter<BoolConverter,Type::Bool> {};
#ifndef PB_WITHOUT_64BIT
        class Int64Converter : public AbstractScalarConverter<Int64Converter,Type::Int64> {};
        class SInt64Converter : public AbstractScalarConverter<SInt64Converter,Type::SInt64> {};
        class UInt64Converter : public AbstractScalarConverter<UInt64Converter,Type::UInt64> {};
        class Fixed64Converter : public AbstractScalarConverter<Fixed64Converter,Type::Fixed64> {};
        class SFixed64Converter : public AbstractScalarConverter<SFixed64Converter,Type::SFixed64> {};
        class DoubleConverter : public AbstractScalarConverter<DoubleConverter,Type::Double> {};
#endif
        class StringConverter : public CallbackConverter<StringConverter, std::string> {
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
        };

        class BytesConverter : public CallbackConverter<BytesConverter, std::string> {
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
        };

        /**
         * Array converter for items
         *
         * @tparam ITEM_CONVERTER
         * @tparam CONTAINER can be std::vector<ITEM_CONVERTER::LocalType> or std::ITEM_CONVERTER::LocalType>
         *
         * NOTE: ITEM_CONVERTER::LocalType and CONTAINER::value_type should match each other
         */
        template<class ITEM_CONVERTER, class CONTAINER>
        class ArrayConverter : public CallbackConverter<ArrayConverter<ITEM_CONVERTER, CONTAINER>,CONTAINER> {
            static_assert(std::is_same<typename ITEM_CONVERTER::LocalType, typename CONTAINER::value_type>::value,
                    "ITEM_CONVERTER::LocalType and CONTAINER::value_type should be same type");
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (const auto &item: container) {
                    if (!ITEM_CONVERTER::encodeCallback(stream, field, item))
                        return false;
                }
                return true;
            }
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                typename ITEM_CONVERTER::LocalType item;
                if (!ITEM_CONVERTER::decodeCallback(stream, field, item))
                    return false;
                container.push_back(item);
                return true;
            }
        };


        /**
         * Converter for vector/list of sub-message
         *
         * @tparam CONVERTER - Derived class
         * @tparam CONTEXT_CONTAINER - std::vector<ITEM_CONVERTER::LocalType> or std::list<ITEM_CONVERTER::LocalType>
         * @tparam ITEM_CONVERTER - MessageConverter<...>
         */
        template<class CONVERTER, class CONTEXT_CONTAINER, class ITEM_CONVERTER>
        class ArrayMessageConverter : public CallbackConverter<
                ArrayMessageConverter<CONVERTER, CONTEXT_CONTAINER, ITEM_CONVERTER>,
                CONTEXT_CONTAINER>
        {
        private:
            using ContextItem = typename CONTEXT_CONTAINER::value_type;
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTEXT_CONTAINER &container){
                for (auto &item: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;
                    if (!encodeSubMessage<ITEM_CONVERTER>(*stream, item))
                        return false;
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTEXT_CONTAINER &container){
                container.push_back(ContextItem());
                ContextItem& v = *container.rbegin();
                if (!decode<ITEM_CONVERTER>(*stream, v))
                    return false;
                return true;
            }
        };

        /**
         * Converter for map
         *
         * @tparam CONVERTER - Derived class
         * @tparam CONTAINER - std::map<> of any type
         * @tparam PROTO_PAIR_TYPE - NanoPb XXX_xxxEntry struct, where xxx is map fild
         * @tparam PROTO_PAIR_TYPE_MSG - NanoPb msg descriptor for PROTO_PAIR_TYPE
         */
        template<class CONVERTER, class CONTAINER, class PROTO_PAIR_TYPE, const pb_msgdesc_t* PROTO_PAIR_TYPE_MSG>
        class MapConverter : public CallbackConverter<
                MapConverter<CONVERTER, CONTAINER, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
                CONTAINER>
        {
        protected:
            using LocalKeyType = typename CONTAINER::key_type;
            using LocalValueType = typename CONTAINER::mapped_type;
            using ProtoPairType = PROTO_PAIR_TYPE;

        private:
            using ContextPairType = std::pair<LocalKeyType,LocalValueType>;

        public:

            static ProtoPairType itemEncoderInit(const LocalKeyType& localKey, const LocalValueType& localValue);
            static ProtoPairType itemDecoderInit(LocalKeyType& localKey, LocalValueType& localValue);
            static bool itemDecoderApply(const ProtoPairType& proto, LocalKeyType& localKey, LocalValueType& localValue);

        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &pair: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    const LocalKeyType& localKey = pair.first;
                    const LocalValueType& localValue = pair.second;

                    ProtoPairType protoPair = CONVERTER::itemEncoderInit(localKey, localValue);

                    if (!pb_encode_submessage(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                        return false;
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                LocalKeyType localKey;
                LocalValueType localValue;

                ProtoPairType protoPair = CONVERTER::itemDecoderInit(localKey, localValue);

                if (!pb_decode(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                    return false;
                if (!CONVERTER::itemDecoderApply(protoPair, localKey, localValue))
                    return false;

                container.insert(std::move(ContextPairType(std::move(localKey), std::move(localValue))));
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
