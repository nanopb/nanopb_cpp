#ifndef NANOPB_CPP_NANOPB_CPP_H
#define NANOPB_CPP_NANOPB_CPP_H

#include <string>
#include <map>
#include <functional>
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

    namespace Converter {

        /**
         * Abstract converter for basic scalar types
         *
         *  Child class example:
         *
         *      using namespace NanoPb::Converter;
         *      
         *      class SimpleEnumConverter: public AbstractScalarConverter<SimpleEnumConverter, SimpleEnum, ProtoSimpleEnum>
         *      private
         *          friend class AbstractScalarConverter
         *          static ProtoType _encode(const LocalType& arg){};
         *          static LocalType _decode(const ProtoType& arg){};
         *      }
         */
        template<class CONVERTER, class LOCAL_TYPE, class PROTO_TYPE>
        class AbstractScalarConverter {
        public:
            using LocalType = LOCAL_TYPE;
            using ProtoType = PROTO_TYPE;

        public:
            static ProtoType encode(const LocalType& arg){ return CONVERTER::_encode(arg); };
            static LocalType decode(const ProtoType& arg){ return CONVERTER::_decode(arg); };
        };


        /**
         * Abstract Callback converter factory
         *
         *  See StringConverter for the example implementation
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class AbstractCallbackConverter {
        public:
            using LocalType = LOCAL_TYPE;
        public:
            static pb_callback_t encoder(const LocalType* arg) { return { .funcs = { .encode = _encodeCallback }, .arg = (void*)arg }; }
            static pb_callback_t decoder(LocalType* arg) { return { .funcs = { .decode = _decodeCallback }, .arg = (void*)arg }; }

        private:
            static bool _encodeCallback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::_encode(stream, field, static_cast<const LocalType*>(*arg));
            };
            static bool _decodeCallback(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::_decode(stream, field, static_cast<LocalType*>(*arg));
            };
        };

        /**
         * StringConverter
         */
        class StringConverter : public AbstractCallbackConverter<StringConverter, std::string> {
        private:
            friend class AbstractCallbackConverter;
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LocalType *arg);
            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LocalType *arg);
        };

        /**
         * AbstractRepeatedConverter
         */
        template<class CONVERTER, class LOCAL_TYPE>
        class AbstractRepeatedConverter : public AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, LOCAL_TYPE>,LOCAL_TYPE>
        {
        public:
            using LocalType = LOCAL_TYPE;
            using LocalEntryType = typename LocalType::value_type;

        private:
            friend class AbstractCallbackConverter<AbstractRepeatedConverter<CONVERTER, LOCAL_TYPE>,LOCAL_TYPE>;

            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LocalType *arg){
                for (auto &item: *arg) {
                    if (!CONVERTER::_encodeItem(stream, field, item)){
                        return false;
                    }
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, LocalType *arg){
                return CONVERTER::_decodeItem(stream, field, arg);
            }
        };

        /**
         * Repeated converter.
         *  Value size depend o PB_WITHOUT_64BIT:
         *      is set: max 32 bit
         *      is not set: max 64 bit
          *
          * @tparam CONTAINER - can be std::vector<XXX> or std::list<XXX>
          */
        template<class CONTAINER>
        class RepeatedUnsignedConverter  : public AbstractRepeatedConverter<
                RepeatedUnsignedConverter<CONTAINER>,
                CONTAINER>
        {
        private:
            using LocalType = CONTAINER;
            using LocalEntryType = typename LocalType::value_type;

            friend class AbstractRepeatedConverter<RepeatedUnsignedConverter<CONTAINER>,CONTAINER>;

            static bool _encodeItem(pb_ostream_t *stream, const pb_field_t *field, const LocalEntryType& item){
                if (!pb_encode_tag_for_field(stream, field)) {
                    return false;
                }
                if (!pb_encode_varint(stream, item))
                    return false;
                return true;
            }

            static bool _decodeItem(pb_istream_t *stream, const pb_field_t *field, LocalType *arg){
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
                arg->push_back(value);
                return true;
            }
        };

        /**
         * AbstractMapConverter
         */
        template<class CONVERTER, class MAP, class PROTO_MAP_ENTRY, const pb_msgdesc_t* PROTO_MAP_ENTRY_MSG>
        class AbstractMapConverter : public AbstractCallbackConverter<AbstractMapConverter<CONVERTER, MAP, PROTO_MAP_ENTRY, PROTO_MAP_ENTRY_MSG>,MAP>
        {
        protected:
            using KeyType = typename MAP::key_type;
            using ValueType = typename MAP::mapped_type;
            using PairType = typename MAP::value_type;

        public:
            using LocalType = MAP;
            using LocalMapPair = typename MAP::value_type;
            using ProtoMapEntry = PROTO_MAP_ENTRY;
        private:
            friend class AbstractCallbackConverter<AbstractMapConverter<CONVERTER, MAP, ProtoMapEntry, PROTO_MAP_ENTRY_MSG>,MAP>;
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, const LocalType *arg){
                for (auto &kv: *arg) {
                    auto &key = kv.first;
                    auto &value = kv.second;

                    ProtoMapEntry entry = CONVERTER::_encoderInitializer(key, value);

                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!pb_encode_submessage(stream, PROTO_MAP_ENTRY_MSG, &entry))
                        return false;
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, __attribute__((unused)) const pb_field_t *field, LocalType *arg){
                KeyType key;
                ValueType value;
                ProtoMapEntry entry = CONVERTER::_decoderInitializer(key, value);
                if (!pb_decode(stream, PROTO_MAP_ENTRY_MSG, &entry)) {
                    return false;
                }
                arg->insert(CONVERTER::_decoderCreateMapPair(entry, key, value));

                return true;
            }
        };
    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
