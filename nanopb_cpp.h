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
        * GenericMapConverter
        */
        template<class MAP, class PROTO_MAP_ENTRY, const pb_msgdesc_t* PROTO_MAP_ENTRY_MSG>
        class GenericMapConverter
        {
        private:
            using MapType = MAP;

            using KeyType = typename MapType::key_type;
            using ValueType = typename MapType::mapped_type;
            using PairType = typename MapType::value_type;

            using EncoderEntryInitializerFunction = std::function<PROTO_MAP_ENTRY(const KeyType&, const ValueType&)>;
            using DecoderEntryInitializerFunction = std::function<PROTO_MAP_ENTRY(KeyType&, ValueType&)>;
            using DecoderMapPairFunction = std::function<PairType(const PROTO_MAP_ENTRY& entry, const KeyType&, const ValueType&)>;

            template<typename CONTEXT_MAP_PTR, class INITIALIZER>
            struct Context {
                CONTEXT_MAP_PTR map;
                INITIALIZER initializerFunction;
                Context(CONTEXT_MAP_PTR map, const INITIALIZER &initializer) :
                        map(map), initializerFunction(initializer) {}
            };
        public:
            using ProtoMapEntry = PROTO_MAP_ENTRY;
            using LocalMapPair = PairType;
            using EncoderContext = Context<const MapType*, EncoderEntryInitializerFunction>;

            struct DecoderContext : public Context<MapType*, DecoderEntryInitializerFunction> {
                DecoderMapPairFunction mapPairFunction;
                DecoderContext(MapType *map,
                               const DecoderEntryInitializerFunction &initializerFunction, const DecoderMapPairFunction& mapPairFunction)
                        : Context<MapType *, DecoderEntryInitializerFunction>(map, initializerFunction), mapPairFunction(mapPairFunction) {}
            };

            static pb_callback_t encoder(const EncoderContext* arg) { return { .funcs = { .encode = _encode }, .arg = (void*)arg }; }
            static pb_callback_t decoder(DecoderContext* arg) { return { .funcs = { .decode = _decode }, .arg = (void*)arg }; }
        private:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                auto ctx = static_cast<const EncoderContext*>(*arg);
                for (auto &kv: *ctx->map) {
                    auto &key = kv.first;
                    auto &value = kv.second;

                    PROTO_MAP_ENTRY entry = ctx->initializerFunction(key, value);

                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!pb_encode_submessage(stream, PROTO_MAP_ENTRY_MSG, &entry))
                        return false;
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, __attribute__((unused)) const pb_field_t *field, void **arg){
                auto ctx = static_cast<DecoderContext*>(*arg);
                KeyType key;
                ValueType value;
                PROTO_MAP_ENTRY entry = ctx->initializerFunction(key, value);
                if (!pb_decode(stream, PROTO_MAP_ENTRY_MSG, &entry)) {
                    return false;
                }
                ctx->map->insert(ctx->mapPairFunction(entry, key, value));

                return true;
            }
        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
