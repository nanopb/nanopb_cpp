#ifndef NANOPB_CPP_NANOPB_CPP_H
#define NANOPB_CPP_NANOPB_CPP_H

#include <string>
#include <memory>

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

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
    bool encode(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::Context& ctx){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);
        return pb_encode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeSubMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::Context& ctx){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(ctx);
        return pb_encode_submessage(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Decode message
     */
    template<class MESSAGE_CONVERTER>
    bool decode(pb_istream_t &stream, typename MESSAGE_CONVERTER::Context& ctx){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::decoderInit(ctx);
        if (!pb_decode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto))
            return false;
        if (!MESSAGE_CONVERTER::decoderApply(proto, ctx))
            return false;
        return true;
    }

    namespace Converter {

        /**
         * Abstract converter for basic scalar types like enum
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE>
        class AbstractScalarConverter {
        protected:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:
            static ProtoType encode(const LocalType& arg){ return CONVERTER::_encode(arg); };
            static LocalType decode(const ProtoType& arg){ return CONVERTER::_decode(arg); };
        };


        /**
         * Abstract proto message converter
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class AbstractMessageConverter {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

            using Context = LocalType;
        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

            static ProtoType encoderInit(const LocalType& local){ return CONVERTER::_encoderInit(local); };
            static ProtoType decoderInit(LocalType& local){ return CONVERTER::_decoderInit(local); };
            static bool decoderApply(const ProtoType& proto, LocalType& local){ return CONVERTER::_decoderApply(proto, local); };
        };

        /**
         * Abstract proto message converter with custom context
         */
        template<class CONVERTER, class CONTEXT, class PROTO_TYPE, const pb_msgdesc_t* PROTO_TYPE_MSG>
        class AbstractMessageContextConverter {
        public:
            using Context = CONTEXT;
            using ProtoType = PROTO_TYPE;
        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

            static ProtoType encoderInit(const Context& ctx){ return CONVERTER::_encoderInit(ctx); };
            static ProtoType decoderInit(Context& ctx){ return CONVERTER::_decoderInit(ctx); };
            static bool decoderApply(const ProtoType& proto, Context& ctx){ return CONVERTER::_decoderApply(proto, ctx); };
        };

        /**
         * Abstract Callback converter
         *
         *  See StringConverter for the example implementation
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class AbstractCallbackConverter {
        protected:
            using LocalType = LOCAL_TYPE;
        public:
            static pb_callback_t encoder(const LocalType& arg) { return { .funcs = { .encode = _encodeCallback }, .arg = (void*)&arg }; }
            static pb_callback_t decoder(LocalType& arg) { return { .funcs = { .decode = _decodeCallback }, .arg = (void*)&arg }; }

        private:
            static bool _encodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::_encode(stream, field, *(static_cast<const LocalType*>(*arg)));
            };
            static bool _decodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::_decode(stream, field, *(static_cast<LocalType*>(*arg)));
            };
        };

        /**
         * StringConverter
         */
        class StringConverter : public AbstractCallbackConverter<StringConverter, std::string> {
        public: // make public to use it from ArrayStringConverter
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LocalType &arg);
            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LocalType &arg);
        };

        /**
         * Abstract repeated converter
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class AbstractRepeatedConverter : public AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, LOCAL_TYPE>,LOCAL_TYPE>
        {
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LOCAL_TYPE &arg){
                for (auto &item: arg) {
                    if (!CONVERTER::_encodeItem(stream, field, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LOCAL_TYPE &arg){
                return CONVERTER::_decodeItem(stream, field, arg);
            }
        };

        /**
         * Array unsigned converter.
         *
         *  Value size depend o PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<XXX> or std::list<XXX>
         */
        template<class CONTAINER>
        class ArrayUnsignedConverter  : public AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER> {
        private:
            using LocalItemType = typename CONTAINER::value_type;
        public:
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const LocalItemType& item){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, item))
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
         *  Value size depend o PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
         *
         * @tparam CONTAINER - can be std::vector<XXX> or std::list<XXX>
         */
        template<class CONTAINER>
        class ArraySignedConverter : public AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER> {
        private:
            using LocalItemType = typename CONTAINER::value_type;
        public:
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const LocalItemType& item){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, item))
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
         */
        template<class CONTAINER>
        class ArrayStringConverter : public AbstractRepeatedConverter<ArrayStringConverter<CONTAINER>,CONTAINER> {
        private:
            using LocalItemType = typename CONTAINER::value_type;
        public:
            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const LocalItemType& item){
                return StringConverter::_encode(stream, field, item);
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
        template<class CONVERTER, class LOCAL_CONTAINER_TYPE, class ITEM_MESSAGE_CONVERTER>
        class ArrayMessageConverter : public AbstractCallbackConverter<
                ArrayMessageConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                LOCAL_CONTAINER_TYPE
        >
        {
        private:
            using LocalItemType = typename ITEM_MESSAGE_CONVERTER::LocalType;
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LOCAL_CONTAINER_TYPE &container){
                for (auto &item: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!encodeSubMessage<ITEM_MESSAGE_CONVERTER>(*stream, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LOCAL_CONTAINER_TYPE &container){
                container.push_back(LocalItemType());
                LocalItemType& localEntry = *container.rbegin();
                if (!decode<ITEM_MESSAGE_CONVERTER>(*stream, localEntry)){
                    return false;
                }
                return true;
            }
        };

        /**
         * Converter for map
         */
        template<class CONVERTER, class LOCAL_CONTAINER_TYPE, class PROTO_PAIR_TYPE, const pb_msgdesc_t* PROTO_PAIR_TYPE_MSG>
        class AbstractMapConverter : public AbstractCallbackConverter<
                AbstractMapConverter<CONVERTER, LOCAL_CONTAINER_TYPE, PROTO_PAIR_TYPE, PROTO_PAIR_TYPE_MSG>,
                LOCAL_CONTAINER_TYPE
        >
        {
        protected:
            using LocalKeyType = typename LOCAL_CONTAINER_TYPE::key_type;
            using LocalValueType = typename LOCAL_CONTAINER_TYPE::mapped_type;
            using ProtoPairType = PROTO_PAIR_TYPE;
        private:
            using LocalPairType = std::pair<LocalKeyType,LocalValueType>;
        public:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LOCAL_CONTAINER_TYPE &container){
                for (auto &pair: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    ProtoPairType protoPair = CONVERTER::_encoderInit(pair.first, pair.second);

                    if (!pb_encode_submessage(stream, PROTO_PAIR_TYPE_MSG, &protoPair)) {
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LOCAL_CONTAINER_TYPE &container){
                LocalKeyType key;
                LocalValueType value;
                ProtoPairType protoPair = CONVERTER::_decoderInit(key, value);

                if (!pb_decode(stream, PROTO_PAIR_TYPE_MSG, &protoPair))
                    return false;
                if (!CONVERTER::_decoderApply(protoPair, key, value))
                    return false;

                container.insert(std::move(LocalPairType(std::move(key), std::move(value))));
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
