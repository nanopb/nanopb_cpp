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
    bool encode(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(local);
        return pb_encode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Encode sub message
     */
    template<class MESSAGE_CONVERTER>
    bool encodeSubMessage(pb_ostream_t &stream, const typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::encoderInit(local);
        return pb_encode_submessage(&stream, MESSAGE_CONVERTER::getMsgType(), &proto);
    }

    /**
     * Decode message
     */
    template<class MESSAGE_CONVERTER>
    bool decode(pb_istream_t &stream, typename MESSAGE_CONVERTER::LocalType& local){
        using ProtoType = typename MESSAGE_CONVERTER::ProtoType;
        ProtoType proto = MESSAGE_CONVERTER::decoderInit(local);
        if (!pb_decode(&stream, MESSAGE_CONVERTER::getMsgType(), &proto))
            return false;
        if (!MESSAGE_CONVERTER::decoderApply(proto, local))
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

        public:
            static const pb_msgdesc_t *getMsgType(){ return PROTO_TYPE_MSG; }

            static ProtoType encoderInit(const LocalType& local){ return CONVERTER::_encoderInit(local); };
            static ProtoType decoderInit(LocalType& local){ return CONVERTER::_decoderInit(local); };
            static bool decoderApply(const ProtoType& proto, LocalType& local){ return CONVERTER::_decoderApply(proto, local); };
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
        public:
            class StringConverterFriend {};
        private:
            friend class AbstractCallbackConverter;

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
        private:
            friend class AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, LOCAL_TYPE>,LOCAL_TYPE>;

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
        private:
            friend class AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER>;

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
        private:
            friend class AbstractRepeatedConverter<ArrayUnsignedConverter<CONTAINER>,CONTAINER>;

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
        private:
            friend class AbstractRepeatedConverter<ArrayStringConverter<CONTAINER>,CONTAINER>;

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
         * Abstract repeated message converter
         */
        template<class CONVERTER, class LOCAL_CONTAINER_TYPE, class ITEM_MESSAGE_CONVERTER>
        class AbstractRepeatedMessageConverter : public AbstractCallbackConverter<
                AbstractRepeatedMessageConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                LOCAL_CONTAINER_TYPE
                >
        {
        private:
            using LocalItemType = typename ITEM_MESSAGE_CONVERTER::LocalType;
            using ProtoItemType = typename ITEM_MESSAGE_CONVERTER::ProtoType;
        private:
            friend class AbstractCallbackConverter<
                    AbstractRepeatedMessageConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                    LOCAL_CONTAINER_TYPE
                    >;

            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LOCAL_CONTAINER_TYPE &container){
                for (const LocalItemType &item: container) {
                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!encodeSubMessage<ITEM_MESSAGE_CONVERTER>(*stream, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LOCAL_CONTAINER_TYPE &container){
                return CONVERTER::_decode(stream, field, container);
            }
        };

        /**
         * Converter for vector/list
         */
        template<class CONVERTER, class LOCAL_CONTAINER_TYPE, class ITEM_MESSAGE_CONVERTER>
        class ArrayMessageConverter : public AbstractRepeatedMessageConverter<
                ArrayMessageConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                LOCAL_CONTAINER_TYPE,
                ITEM_MESSAGE_CONVERTER
        >
        {
        private:
            using LocalItemType = typename ITEM_MESSAGE_CONVERTER::LocalType;
        private:
            friend class AbstractRepeatedMessageConverter<
                    ArrayMessageConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                    LOCAL_CONTAINER_TYPE,
                    ITEM_MESSAGE_CONVERTER
            >;

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
        template<class CONVERTER, class LOCAL_CONTAINER_TYPE, class ITEM_MESSAGE_CONVERTER>
        class AbstractMapConverter : public AbstractRepeatedMessageConverter<
                AbstractMapConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                LOCAL_CONTAINER_TYPE,
                ITEM_MESSAGE_CONVERTER
        >
        {
        private:
            using LocalItemType = typename ITEM_MESSAGE_CONVERTER::LocalType;
        private:
            friend class AbstractRepeatedMessageConverter<
                    AbstractMapConverter<CONVERTER, LOCAL_CONTAINER_TYPE, ITEM_MESSAGE_CONVERTER>,
                    LOCAL_CONTAINER_TYPE,
                    ITEM_MESSAGE_CONVERTER
            >;

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LOCAL_CONTAINER_TYPE &container){
                LocalItemType localEntry;
                if (!decode<ITEM_MESSAGE_CONVERTER>(*stream, localEntry)){
                    return false;
                }
                container.insert(localEntry);
                return true;
            }

        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
