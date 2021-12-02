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

        const LocalType& ctx = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);

        return pb_encode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeSubMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& v){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using LocalType = typename MESSAGE_CONVERTER::LocalType;

        const LocalType& ctx = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);

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

        const LocalType& ctx = v;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);

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

        LocalType& ctx = v;
        ProtoType proto = MESSAGE_CONVERTER::decoderInit(ctx);

        if (!pb_decode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto))
            return false;
        if (!MESSAGE_CONVERTER::decoderApply(proto, ctx))
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
         */
        template<class LOCAL_TYPE, class PROTO_TYPE>
        class AbstractScalarConverter {
        protected:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:  // Should be overwritten in child class

            static ProtoType encode(const LocalType& arg);
            static LocalType decode(const ProtoType& arg);
        };

        /**
         * Abstract message converter
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class AbstractMessageConverter {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

        public:  // Should be overwritten in child class

            static ProtoType encoderInit(const LocalType& ctx);
            static ProtoType decoderInit(LocalType& ctx);
            static bool decoderApply(const ProtoType& proto, LocalType& ctx);
        };

        /**
         * Abstract union message converter
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class AbstractUnionMessageConverter : public AbstractMessageConverter<CONVERTER, LOCAL_TYPE, PROTO_TYPE, PROTO_TYPE_MSG>{
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

            static pb_callback_t unionDecoderInit(LocalType& ctx) { return pb_callback_t{ .funcs = { .decode = _unionDecodeCallback }, .arg = (void*)&ctx }; }

        public:  // Should be overwritten in child class

            static ProtoType encoderInit(const LocalType& ctx);
            static ProtoType decoderInit(LocalType& ctx);
            static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &ctx);
            static bool decoderApply(const ProtoType& proto, LocalType& ctx);

        private:
            static bool _unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::unionDecodeCallback(stream, field, *(static_cast<LocalType *>(*arg)));
            }
        };

        /**
         * Abstract Callback converter
         *
         *  See StringConverter for the example implementation
         */
        template<class CONVERTER, class CONTEXT>
        class AbstractCallbackConverter {
        protected:
            using Context = CONTEXT;
        public:
            static pb_callback_t encoderInit(const Context& ctx) { return pb_callback_t{ .funcs = { .encode = _encodeCallback }, .arg = (void*)&ctx }; }
            static pb_callback_t decoderInit(Context& ctx) { return pb_callback_t{ .funcs = { .decode = _decodeCallback }, .arg = (void*)&ctx }; }

        public:  // Should be overwritten in child class

            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const Context &arg);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, Context &arg);

        private:
            static bool _encodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::encodeCallback(stream, field, *(static_cast<const Context *>(*arg)));
            };
            static bool _decodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::decodeCallback(stream, field, *(static_cast<Context *>(*arg)));
            };
        };

        /**
         * StringConverter
         * Can be used to encode/decode string and bytes fields to/from std::string
         */
        class StringConverter : public AbstractCallbackConverter<StringConverter, std::string> {
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const Context &arg);
            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, Context &arg);
        };

        /**
         * Abstract repeated converter
         */
        template<class CONVERTER, class CONTAINER>
        class AbstractRepeatedConverter : public AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, CONTAINER>,CONTAINER> {
        private:
            using ValueType = typename CONTAINER::value_type;
        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &item: container) {
                    if (!CONVERTER::encodeItem(stream, field, item)){
                        return false;
                    }
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
         *  Value size depend on PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<XXX> or std::list<XXX>
         */
        template<class CONTAINER>
        class ArrayUnsignedConverter  : public AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER> {
        private:
            using UnsignedType = typename CONTAINER::value_type;
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const UnsignedType& number){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, number))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER & container){
#ifdef PB_WITHOUT_64BIT
                uint32_t value;
                if (!pb_decode_varint32(stream, &value)) {
                    return false;
                }
#else
                uint64_t value;
                if (!pb_decode_varint(stream, &value)) {
                    return false;
                }
#endif
                container.push_back(value);
                return true;
            }
        };

        /**
         * Array signed converter.
         *
         *  Value size depend on PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<XXX> or std::list<XXX>
         */
        template<class CONTAINER>
        class ArraySignedConverter : public AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER> {
        private:
            using SignedType = typename CONTAINER::value_type;
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const SignedType& number){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, number))
                    return false;
                return true;
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
#ifdef PB_WITHOUT_64BIT
                int32_t value;
                if (!pb_decode_svarint32(stream, &value)) {
                    return false;
                }
#else
                int64_t value;
                if (!pb_decode_svarint(stream, &value)) {
                    return false;
                }
#endif
                container.push_back(value);
                return true;
            }
        };

        /**
         * Array string converter.
         *
         * @tparam CONTAINER - can be std::vector<std::string> or std::list<std::string>
         */
        template<class CONTAINER>
        class ArrayStringConverter : public AbstractRepeatedConverter<ArrayStringConverter<CONTAINER>,CONTAINER> {
        public:
            static bool encodeItem(pb_ostream_t *stream, const pb_field_t *field, const std::string& str){
                return StringConverter::encodeCallback(stream, field, str);
            }

            static bool decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
                std::string str;
                if  (!StringConverter::decodeCallback(stream, field, str))
                    return false;
                container.push_back(str);
                return true;
            }
        };

        /**
         * Converter for vector/list
         */
        template<class CONVERTER, class CONTEXT_CONTAINER, class ITEM_CONVERTER>
        class ArrayMessageConverter : public AbstractCallbackConverter<
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

                    if (!encodeSubMessage<ITEM_CONVERTER>(*stream, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTEXT_CONTAINER &container){
                container.push_back(ContextItem());
                ContextItem& v = *container.rbegin();
                if (!decode<ITEM_CONVERTER>(*stream, v)){
                    return false;
                }
                return true;
            }
        };

        /**
         * Converter for map
         *
         *  ItemEncoderContext/ItemDecoderContext used as initializers inside NanoPb::decode().
         *      This contexts can be overwritten for complicated types in child classed.
         */
        template<class CONVERTER, class CONTAINER, class PROTO_PAIR_TYPE, const pb_msgdesc_t* PROTO_PAIR_TYPE_MSG>
        class MapConverter : public AbstractCallbackConverter<
                MapConverter<CONVERTER, CONTAINER, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
                CONTAINER>
        {
        protected:
            using KeyType = typename CONTAINER::key_type;
            using ValueType = typename CONTAINER::mapped_type;
            using ProtoPairType = PROTO_PAIR_TYPE;

            struct ItemEncoderContext {
                const KeyType& key;
                const ValueType& value;
                ItemEncoderContext(const KeyType &key, const ValueType &value) : key(key), value(value) {}
            };

            struct ItemDecoderContext {
                KeyType& key;
                ValueType& value;
                ItemDecoderContext(KeyType &key, ValueType &value) : key(key), value(value) {}
            };
        private:
            using ContextPairType = std::pair<KeyType,ValueType>;

        public:

            static ProtoPairType itemEncoderInit(const ItemEncoderContext& ctx);
            static ProtoPairType itemDecoderInit(ItemDecoderContext& ctx);
            static bool itemDecoderApply(const ProtoPairType& proto, ItemDecoderContext& ctx);

        public:
            static bool encodeCallback(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &pair: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    typename CONVERTER::ItemEncoderContext ctx(pair.first, pair.second);

                    ProtoPairType protoPair = CONVERTER::itemEncoderInit(ctx);

                    if (!pb_encode_submessage(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                        return false;
                }
                return true;
            }

            static bool decodeCallback(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                KeyType key;
                ValueType value;

                typename CONVERTER::ItemDecoderContext ctx(key, value);

                ProtoPairType protoPair = CONVERTER::itemDecoderInit(ctx);

                if (!pb_decode(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                    return false;
                if (!CONVERTER::itemDecoderApply(protoPair, ctx))
                    return false;

                container.insert(std::move(ContextPairType(std::move(key), std::move(value))));
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
