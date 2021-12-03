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


    namespace Converter {

        /**
         * Abstract converter for basic scalar types like enum
         *
         * @tparam LOCAL_TYPE - Local type
         * @tparam PROTO_TYPE - NanoPb type
         */
        template<class LOCAL_TYPE, class PROTO_TYPE>
        class ScalarConverter {
        protected:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:  // Should be overwritten in child class

            static ProtoType encode(const LocalType& local);
            static LocalType decode(const ProtoType& proto);
        };

        /**
         * Abstract message converter
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
         * Abstract union message converter
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
         * Abstract Callback converter
         *
         *  See StringCallbackConverter for the example implementation
         *
         * @tparam CONVERTER - Derived class
         * @tparam LOCAL_TYPE - Local type
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class CallbackConverter {
        protected:
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
         * StringCallbackConverter
         * Can be used to encode/decode string and bytes fields to/from std::string
         */
        class StringCallbackConverter : public CallbackConverter<StringCallbackConverter, std::string> {
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const LocalType &local);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local);
        };

        /**
         * Abstract repeated converter
         *
         * @tparam CONVERTER - Derived class
         * @tparam CONTAINER - Local container type
         */
        template<class CONVERTER, class CONTAINER>
        class RepeatedCallbackConverter : public CallbackConverter<RepeatedCallbackConverter<CONVERTER, CONTAINER>,CONTAINER> {
        private:
            using ValueType = typename CONTAINER::value_type;
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &item: container) {
                    if (!CONVERTER::encodeItem(stream, field, item))
                        return false;
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                return CONVERTER::decodeItem(stream, field, container);
            }

        public:  // Should be overwritten in child class
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const ValueType& item);
            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER & container);
        };

        /**
         * Array unsigned converter.
         *
         *  VALUE: unsigned|uint64_t|uint32_t|uint16_t|uint8_t|...
         *
         *  Value size depend on PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<VALUE> or std::list<VALUE>
         */
        template<class CONTAINER>
        class ArrayUnsignedCallbackConverter  : public RepeatedCallbackConverter<ArrayUnsignedCallbackConverter<CONTAINER>,CONTAINER> {
        private:
            using ValueType = typename CONTAINER::value_type;
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const ValueType& number){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                if (!pb_encode_varint(stream, number))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER & container){
#ifdef PB_WITHOUT_64BIT
                uint32_t value;
                if (!pb_decode_varint32(stream, &value))
                    return false;
#else
                uint64_t value;
                if (!pb_decode_varint(stream, &value))
                    return false;
#endif
                container.push_back(value);
                return true;
            }
        };

        /**
         * Array signed converter.
         *
         *  VALUE: int|int64_t|int32_t|int16_t|int8_t|...
         *
         *  Value size depend on PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<VALUE> or std::list<VALUE>
         */
        template<class CONTAINER>
        class ArraySignedCallbackConverter : public RepeatedCallbackConverter<ArraySignedCallbackConverter<CONTAINER>,CONTAINER> {
        private:
            using ValueType = typename CONTAINER::value_type;
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const ValueType& number){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
#ifdef PB_WITHOUT_64BIT
                int32_t value = number;
#else
                int64_t value = number;
#endif
                if (!pb_encode_svarint(stream, value))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
#ifdef PB_WITHOUT_64BIT
                int32_t value;
#else
                int64_t value;
#endif
                if (!pb_decode_svarint(stream, &value))
                    return false;
                container.push_back(value);
                return true;
            }
        };

        /**
         * Array float converter.
         *
         * @tparam CONTAINER - can be std::vector<float> or std::list<float>
         */
        template<class CONTAINER>
        class ArrayFloatCallbackConverter : public RepeatedCallbackConverter<ArrayFloatCallbackConverter<CONTAINER>,CONTAINER> {
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const float& number){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                if (!pb_encode_fixed32(stream, &number))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
                float value;
                if (!pb_decode_fixed32(stream, &value))
                    return false;
                container.push_back(value);
                return true;
            }
        };

#ifndef PB_WITHOUT_64BIT
        /**
         * Array double converter.
         *
         * @tparam CONTAINER - can be std::vector<float> or std::list<float>
         */
        template<class CONTAINER>
        class ArrayDoubleCallbackConverter : public RepeatedCallbackConverter<ArrayDoubleCallbackConverter<CONTAINER>,CONTAINER> {
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const double& number){
                if (!pb_encode_tag_for_field(stream, field))
                    return false;
                if (!pb_encode_fixed64(stream, &number))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
                double value;
                if (!pb_decode_fixed64(stream, &value))
                    return false;
                container.push_back(value);
                return true;
            }
        };
#endif


        /**
         * Array string converter.
         *
         * @tparam CONTAINER - can be std::vector<std::string> or std::list<std::string>
         */
        template<class CONTAINER>
        class ArrayStringCallbackConverter : public RepeatedCallbackConverter<ArrayStringCallbackConverter<CONTAINER>,CONTAINER> {
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const std::string& str){
                return StringCallbackConverter::encodeCallback(stream, field, str);
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
                std::string str;
                if  (!StringCallbackConverter::decodeCallback(stream, field, str))
                    return false;
                container.push_back(str);
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
        class ArrayMessageCallbackConverter : public CallbackConverter<
                ArrayMessageCallbackConverter<CONVERTER, CONTEXT_CONTAINER, ITEM_CONVERTER>,
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
        class MapCallbackConverter : public CallbackConverter<
                MapCallbackConverter<CONVERTER, CONTAINER, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
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
