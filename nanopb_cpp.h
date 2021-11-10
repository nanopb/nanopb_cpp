#ifndef NANOPB_CPP_NANOPB_CPP_H
#define NANOPB_CPP_NANOPB_CPP_H

#include <string>
#include <map>

#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

#ifndef NANOPB_CPP_ASSERT
#ifndef NDEBUG
#include <assert.h>
#define NANOPB_CPP_ASSERT(expr) assert(expr)
#else
#define _nanopb_cpp_unused(x) ((void)(x))
#define NANOPB_CPP_ASSERT(expr) _nanopb_cpp_unused(expr)
#endif
#endif

namespace NanoPb {
    namespace Converter {
        /**
         * AbstractSingleArgConverter
         */
        template<class CONVERTER, class ARG>
        class AbstractSingleArgConverter {
        public:
            static pb_callback_t encoder(const ARG* arg) { return { .funcs = { .encode = _encode }, .arg = (void*)arg }; }
            static pb_callback_t decoder(ARG* arg) { return { .funcs = { .decode = _decode }, .arg = (void*)arg }; }

        private:
            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                return CONVERTER::encode(stream, field, static_cast<const ARG*>(*arg));
            };
            static bool _decode(pb_istream_t *stream, const pb_field_t *field, void **arg){
                return CONVERTER::decode(stream, field, static_cast<ARG*>(*arg));
            };
        };

        /**
         * StringConverter
         */
        class StringConverter : public AbstractSingleArgConverter<StringConverter, std::string> {
        private:
            friend class AbstractSingleArgConverter;
            static bool encode(pb_ostream_t *stream, const pb_field_t *field, const std::string *arg);
            static bool decode(pb_istream_t *stream, const pb_field_t *field, std::string *arg);
        };

        /**
        * GenericMapConverter
        */
        template<class MAP, class PROTO_MAP_ENTRY>
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
                const pb_msgdesc_t* msgDesc;
                INITIALIZER initializerFunction;
                Context(CONTEXT_MAP_PTR map, const pb_msgdesc_t *msgDesc, const INITIALIZER &initializer) :
                        map(map), msgDesc(msgDesc), initializerFunction(initializer) {}
            };
        public:
            using EncoderContext = Context<const MapType*, EncoderEntryInitializerFunction>;

            struct DecoderContext : public Context<MapType*, DecoderEntryInitializerFunction> {
                DecoderMapPairFunction mapPairFunction;
                DecoderContext(MapType *map, const pb_msgdesc_t *msgDesc,
                               const DecoderEntryInitializerFunction &initializerFunction, const DecoderMapPairFunction& mapPairFunction)
                        : Context<MapType*, DecoderEntryInitializerFunction>(map, msgDesc, initializerFunction), mapPairFunction(mapPairFunction) {}
            };

            static pb_callback_t encoder(const EncoderContext* arg) { return { .funcs = { .encode = _encode }, .arg = (void*)arg }; }
            static pb_callback_t decoder(DecoderContext* arg) { return { .funcs = { .decode = _decode }, .arg = (void*)arg }; }
        private:
            GenericMapConverter() {}

            static bool _encode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg){
                auto ctx = static_cast<const EncoderContext*>(*arg);
                for (auto &kv: *ctx->map) {
                    auto &key = kv.first;
                    auto &value = kv.second;

                    PROTO_MAP_ENTRY entry = ctx->initializerFunction(key, value);

                    if (!pb_encode_tag_for_field(stream, field))
                        return false;

                    if (!pb_encode_submessage(stream, ctx->msgDesc, &entry))
                        return false;
                }
                return true;
            }

            static bool _decode(pb_istream_t *stream, const pb_field_t *field, void **arg){
                auto ctx = static_cast<DecoderContext*>(*arg);
                KeyType key;
                ValueType value;
                PROTO_MAP_ENTRY entry = ctx->initializerFunction(key, value);
                if (!pb_decode(stream, ctx->msgDesc, &entry)) {
                    return false;
                }
                ctx->map->insert(ctx->mapPairFunction(entry, key, value));

                return true;
            }
        };

    }
}

#endif //NANOPB_CPP_NANOPB_CPP_H
