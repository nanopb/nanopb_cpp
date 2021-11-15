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
    bool encode(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using EncoderContext = typename MESSAGE_CONVERTER::EncoderContext;

        EncoderContext ctx(local);
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);

        return pb_encode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeSubMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using EncoderContext = typename MESSAGE_CONVERTER::EncoderContext;

        EncoderContext ctx(local);
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);

        return pb_encode_submessage(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Decode message
     */
    template<class MESSAGE_CONVERTER>
    bool decode(pb_istream_t &stream, typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        using DecoderContext = typename MESSAGE_CONVERTER::DecoderContext;

        DecoderContext ctx(local);
        ProtoType proto = MESSAGE_CONVERTER::decoderInit(ctx);

        if (!pb_decode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto))
            return false;
        if (!MESSAGE_CONVERTER::decoderApply(proto, ctx))
            return false;
        return true;
    }

    /**
     * Decode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool decodeSubMessage(pb_istream_t &stream, typename MESSAGE_CONVERTER::LocalType& local){
        pb_istream_t subStream;

        if (!pb_make_string_substream(&stream, &subStream))
            return false;

        if (!decode<MESSAGE_CONVERTER>(subStream, local))
            return false;

        if (!pb_close_string_substream(&stream, &subStream))
            return false;
        return true;
    }

    /**
     * Decode first tag from the stream.
     */
    bool decodeTag(pb_istream_t &stream, uint32_t& tag);


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
        class AbstractMessageConverter {public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

            struct EncoderContext {
                const LocalType& local;
                EncoderContext(const LocalType &local) : local(local) {}
            };
            struct DecoderContext {
                LocalType& local;
                DecoderContext(LocalType &local) : local(local) {}
            };
        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

        public:  // Should be overwritten in child class

            static ProtoType encoderInit(const EncoderContext& ctx);
            static ProtoType decoderInit(DecoderContext& ctx);
            static bool decoderApply(const ProtoType& proto, DecoderContext& ctx);

            static EncoderContext createEncoderContext(const LocalType& local);
            static DecoderContext createDecoderContext(LocalType& local);
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
            static pb_callback_t encoder(const Context& ctx) { return { .funcs = { .encode = _encodeCallback }, .arg = (void*)&ctx }; }
            static pb_callback_t decoder(Context& ctx) { return { .funcs = { .decode = _decodeCallback }, .arg = (void*)&ctx }; }

        public:  // Should be overwritten in child class

            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const Context &arg);
            static bool _decode(pb_istream_t *stream, const pb_field_t *field, Context &arg);

        private:
            static bool _encodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::_encode(stream, field, *(static_cast<const Context*>(*arg)));
            };
            static bool _decodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::_decode(stream, field, *(static_cast<Context*>(*arg)));
            };
        };

        /**
         * StringConverter
         */
        class StringConverter : public AbstractCallbackConverter<StringConverter, std::string> {
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const Context &arg);
            static bool _decode(pb_istream_t *stream, const pb_field_t *field, Context &arg);
        };

        /**
         * Abstract repeated converter
         */
        template<class CONVERTER, class CONTAINER>
        class AbstractRepeatedConverter : public AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, CONTAINER>,CONTAINER> {
        private:
            using ValueType = typename CONTAINER::value_type;
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &item: container) {
                    if (!CONVERTER::_encodeItem(stream, field, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                return CONVERTER::_decodeItem(stream, field, container);
            }

        public:  // Should be overwritten in child class
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const ValueType& item);
            static bool _decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER & container);
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
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const UnsignedType& number){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, number))
                    return false;
                return true;
            }

            static bool _decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER & container){
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
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const SignedType& number){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, number))
                    return false;
                return true;
            }

            static bool _decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
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
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const std::string& str){
                return StringConverter::_encode(stream, field, str);
            }

            static bool _decodeItem(pb_istream_t *stream, const pb_field_t *field, CONTAINER& container){
                std::string str;
                if  (!StringConverter::_decode(stream, field, str))
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
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const CONTEXT_CONTAINER &container){
                for (auto &item: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!encodeSubMessage<ITEM_CONVERTER>(*stream, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, CONTEXT_CONTAINER &container){
                container.push_back(ContextItem());
                ContextItem& localEntry = *container.rbegin();
                if (!decode<ITEM_CONVERTER>(*stream, localEntry)){
                    return false;
                }
                return true;
            }
        };

        /**
         * Converter for map
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

            struct EncoderContext {
                const KeyType& key;
                const ValueType& value;
                EncoderContext(const KeyType &key, const ValueType &value) : key(key), value(value) {}
            };

            struct DecoderContext {
                KeyType& key;
                ValueType& value;
                DecoderContext(KeyType &key, ValueType &value) : key(key), value(value) {}
            };
        private:
            using ContextPairType = std::pair<KeyType,ValueType>;
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const CONTAINER &container){
                for (auto &pair: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    typename CONVERTER::EncoderContext ctx(pair.first, pair.second);

                    ProtoPairType protoPair = CONVERTER::encoderInit(ctx);

                    if (!pb_encode_submessage(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                        return false;
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, CONTAINER &container){
                KeyType key;
                ValueType value;

                typename CONVERTER::DecoderContext ctx(key, value);

                ProtoPairType protoPair = CONVERTER::decoderInit(ctx);

                if (!pb_decode(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                    return false;
                if (!CONVERTER::decoderApply(protoPair, ctx))
                    return false;

                container.insert(std::move(ContextPairType(std::move(key), std::move(value))));
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
