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

        /**
         * Create output stream without max size limits.
         * NOTE: Prefer to use `StringOutputStream(size_t maxStreamSize)` constructor to avoid filling all RAM.
         */
        StringOutputStream();

        /**
         * Create output memory stream with constant max size
         *
         * @param maxStreamSize
         */
        StringOutputStream(size_t maxStreamSize);
        BufferPtr release();
    private:
        static bool _pbCallback(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
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
        using DecoderContext = typename MESSAGE_CONVERTER::DecoderContext;

        DecoderContext local = v;
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
     *      encode()/decode()
     *
     */
    namespace Type {
        template<class LOCAL_TYPE>
        class AbstractScalarType {
        public:
            using LocalType = LOCAL_TYPE;
        };

        class Int32 : public AbstractScalarType<int32_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class SInt32 : public AbstractScalarType<int32_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class UInt32 : public AbstractScalarType<uint32_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class Fixed32 : public AbstractScalarType<uint32_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class SFixed32 : public AbstractScalarType<int32_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class Float : public AbstractScalarType<float>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class Bool : public AbstractScalarType<bool>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        /**
         * NOTE: encode()/decode() **does NOT** add length for String/Bytes
         */
        class String : public AbstractScalarType<std::string>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        /**
         * NOTE: encode()/decode() **does NOT** add length for String/Bytes
         */
        class Bytes : public AbstractScalarType<std::string>{ // use std::string as container
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

#ifndef PB_WITHOUT_64BIT
        class Int64 : public AbstractScalarType<int64_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class SInt64 : public AbstractScalarType<int64_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class UInt64 : public AbstractScalarType<uint64_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class Fixed64 : public AbstractScalarType<uint64_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class SFixed64 : public AbstractScalarType<int64_t>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };

        class Double : public AbstractScalarType<double>{
        public:
            static bool encode(pb_ostream_t *stream, const LocalType& value);
            static bool decode(pb_istream_t *stream, LocalType& value);
        };
#endif
    }

    namespace Converter {
        /**
          * Callback converter
          *
          *  Derived class must implement next methods:
          *
          *      static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
          *      static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
          *
          *  NOTE: Usually user must implement `MessageConverter` and NOT extend this class directly.
          *
          * @tparam DERIVED - Derived class
          * @tparam LOCAL_TYPE - Local type
          */
        template<class DERIVED, class LOCAL_TYPE>
        class CallbackConverter {
        public:
            using LocalType = LOCAL_TYPE;
        public:
            static pb_callback_t encoderCallbackInit(const LocalType& local) { return pb_callback_t{ .funcs = { .encode = _pbEncodeCallback }, .arg = (void*)&local }; }
            static pb_callback_t decoderCallbackInit(LocalType& local) { return pb_callback_t{ .funcs = { .decode = _pbDecodeCallback }, .arg = (void*)&local }; }

        private:
            static bool _pbEncodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return DERIVED::encodeCallback(stream, field, *(static_cast<const LocalType *>(*arg)));
            };
            static bool _pbDecodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return DERIVED::decodeCallback(stream, field, *(static_cast<LocalType *>(*arg)));
            };

        public: // for internal use
            template<class T> static void _mapEncoderApply(T& pair){}
        };


        /**
         * Enum converter
         *
         *  Derived class must implement next methods:
         *
         *      static ProtoType encode(const LocalType& local);
         *      static LocalType decode(const ProtoType& proto);
         *
         * @tparam DERIVED - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         */
        template<class DERIVED, class LOCAL_TYPE, class PROTO_TYPE>
        class EnumConverter : public CallbackConverter<
                EnumConverter<DERIVED, LOCAL_TYPE, PROTO_TYPE>,
                LOCAL_TYPE>
        {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                ProtoType v = DERIVED::encode(local);
                return Type::Int32::encode(stream, v);
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
                int32_t v;
                if (!Type::Int32::decode(stream, v))
                    return false;
                local = DERIVED::decode(static_cast<ProtoType>(v));
                return true;
            }
        public:
            static ProtoType encoderInit(const LocalType& local){
                return DERIVED::encode(local);
            }
            static ProtoType decoderInit(LocalType& local){ return ProtoType{}; }
            static bool decoderApply(const ProtoType& proto, LocalType& local){
                local =  DERIVED::decode(proto);
                return true;
            }
        public: // for internal use
            template<class T> static void _mapEncoderApply(T& pair){}
        };


        /**
         * Message converter
         *
         *  Derived class must implement next methods:
         *
         *      static ProtoType encoderInit(const LocalType& local);
         *      static ProtoType decoderInit(LocalType& local);
         *      static bool decoderApply(const ProtoType& proto, LocalType& local);
         *
         * @tparam DERIVED - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         * @tparam PROTO_TYPE_MSG - NanoPb msg descriptor
         */
        template<class DERIVED, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class MessageConverter : public CallbackConverter<
                MessageConverter<DERIVED, LOCAL_TYPE, PROTO_TYPE, PROTO_TYPE_MSG>,
                LOCAL_TYPE>
        {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;
            /**
             * You can define custom DecoderContext.
             * This can be useful to store temporary vars.
             * See examples/comlex/converters.hpp for the details.
             */
            using DecoderContext = LocalType&;

        public:
            static constexpr const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                return encodeSubMessage<DERIVED>(*stream, local);
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
                return decode<DERIVED>(*stream, local);
            }

        public: // for internal use
            template<class T>
            static void _mapEncoderApply(T& pair){ pair.has_value = true; }
        };

        /**
         * Union message converter
         *
         *  Derived class must implement:
         *
         *      - all methods from MessageConverter.
         *      - additional methods:
         *
         *          static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
         *
         * @tparam DERIVED - Derived class
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         * @tparam PROTO_TYPE_MSG - NanoPb msg descriptor
         */
        template<class DERIVED, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class UnionMessageConverter : public MessageConverter<DERIVED, LOCAL_TYPE, PROTO_TYPE, PROTO_TYPE_MSG>{
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

            static pb_callback_t unionDecoderInit(LocalType& local) { return pb_callback_t{ .funcs = { .decode = _unionDecodeCallback }, .arg = (void*)&local }; }

        private:
            static bool _unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return DERIVED::unionDecodeCallback(stream, field, *(static_cast<LocalType *>(*arg)));
            }
        };

        /**
         * Basic scalar converter.
         *
         *  Can be used for field with wire types: PB_WT_VARINT, PB_WT_64BIT, PB_WT_32BIT
         *
         * @tparam DERIVED - Derived class
         * @tparam SCALAR - Scalar with one of listed above wire type.
         */
        template <class DERIVED, class SCALAR>
        class AbstractScalarConverter : public CallbackConverter<DERIVED, typename SCALAR::LocalType> {
        public:
            using LocalType = typename SCALAR::LocalType;
            using ProtoType = typename SCALAR::LocalType; // Proto type for basic scalar is same as local.
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                return SCALAR::encode(stream, local);
            }
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
                return SCALAR::decode(stream, local);
            }
        public:
            static ProtoType encoderInit(const LocalType& local){ return local;}
            static ProtoType decoderInit(LocalType& local){ return ProtoType{}; }
            static bool decoderApply(const ProtoType& proto, LocalType& local){
                local = proto;
                return true;
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
        public:
            static pb_callback_t encoderInit(const LocalType& local){ return encoderCallbackInit(local);}
            static pb_callback_t decoderInit(LocalType& local){ return decoderCallbackInit(local);}
            static bool decoderApply(const pb_callback_t& proto, LocalType& local){ return true;/* nothing to apply */}
        };

        class BytesConverter : public CallbackConverter<BytesConverter, std::string> {
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
        public:
            static pb_callback_t encoderInit(const LocalType& local){ return encoderCallbackInit(local);}
            static pb_callback_t decoderInit(LocalType& local){ return decoderCallbackInit(local);}
            static bool decoderApply(const pb_callback_t& proto, LocalType& local){ return true;/* nothing to apply */}
        };

        /**
         * Array converter for items
         *
         * @tparam ITEM_CONVERTER - Derived from MessageConverter class
         * @tparam CONTAINER can be std::vector<ITEM_CONVERTER::LocalType> or std::ITEM_CONVERTER::LocalType>
         *
         * NOTE: ITEM_CONVERTER::LocalType and CONTAINER::value_type should match each other
         */
        template<class ITEM_CONVERTER, class CONTAINER>
        class ArrayConverter : public CallbackConverter<ArrayConverter<ITEM_CONVERTER, CONTAINER>,CONTAINER>
        {
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
                container.emplace_back(typename ITEM_CONVERTER::LocalType());
                typename ITEM_CONVERTER::LocalType& item = *container.rbegin();
                if (!ITEM_CONVERTER::decodeCallback(stream, field, item))
                    return false;
                return true;
            }
        };

        /**
         * Map converter
         *
         * @tparam KEY_CONVERTER - Key converter
         * @tparam VALUE_CONVERTER - Value converter
         * @tparam CONTAINER - std::map<> of any type
         * @tparam PROTO_PAIR_TYPE - NanoPb XXX_xxxEntry struct, where xxx is map field
         * @tparam PROTO_PAIR_TYPE_MSG - NanoPb msg descriptor for PROTO_PAIR_TYPE
         */
        template<class KEY_CONVERTER, class VALUE_CONVERTER, class CONTAINER, class PROTO_PAIR_TYPE, const pb_msgdesc_t* PROTO_PAIR_TYPE_MSG>
        class MapConverter : public CallbackConverter<
                MapConverter<KEY_CONVERTER, VALUE_CONVERTER, CONTAINER, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
                CONTAINER>
        {
            static_assert(std::is_same<typename KEY_CONVERTER::LocalType, typename CONTAINER::key_type>::value,
                          "KEY_CONVERTER::LocalType and CONTAINER::key_type should be same type");
            static_assert(std::is_same<typename VALUE_CONVERTER::LocalType, typename CONTAINER::mapped_type>::value,
                          "VALUE_CONVERTER::LocalType and CONTAINER::mapped_type should be same type");

        private:
            using LocalKeyType = typename CONTAINER::key_type;
            using LocalValueType = typename CONTAINER::mapped_type;
            using ProtoPairType = PROTO_PAIR_TYPE;
            using ContextPairType = std::pair<LocalKeyType,LocalValueType>;

        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &pair: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    ProtoPairType protoPair {
                        .key = KEY_CONVERTER::encoderInit(pair.first),
                        .value = VALUE_CONVERTER::encoderInit(pair.second)
                    };

                    VALUE_CONVERTER::template _mapEncoderApply<ProtoPairType>(protoPair);

                    if (!pb_encode_submessage(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                        return false;
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                LocalKeyType localKey;
                LocalValueType localValue;

                ProtoPairType protoPair {
                        .key = KEY_CONVERTER::decoderInit(localKey),
                        .value = VALUE_CONVERTER::decoderInit(localValue)
                };

                if (!pb_decode(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                    return false;
                if (!KEY_CONVERTER::decoderApply(protoPair.key, localKey))
                    return false;
                if (!VALUE_CONVERTER::decoderApply(protoPair.value, localValue))
                    return false;

                container.insert(std::move(ContextPairType(std::move(localKey), std::move(localValue))));
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
