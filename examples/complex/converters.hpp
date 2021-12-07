#pragma once

#include "models.hpp"
#include "nanopb_cpp.h"

#include "complex.pb.h"
#include "models.hpp"

using namespace NanoPb::Converter;
using namespace Model;

/**
 * FoodConverter
 */
class FoodConverter: public EnumConverter<FoodConverter, Food, PROTO_Food> {
public:
    static ProtoType encode(const LocalType& local){
        switch (local) {
            case Food::Invalid: return PROTO_Food_Invalid;
            case Food::Meat: return PROTO_Food_Meat;
            case Food::Chicken: return PROTO_Food_Chicken;
            case Food::Vegetable: return PROTO_Food_Vegetable;
        }
        return PROTO_Food_Invalid;
    };
    static LocalType decode(const ProtoType& proto){
        switch (proto) {
            case PROTO_Food_Invalid: return Food::Invalid;
            case PROTO_Food_Meat: return Food::Meat;
            case PROTO_Food_Chicken: return Food::Chicken;
            case PROTO_Food_Vegetable: return Food::Vegetable;
        }
        return Food::Invalid;
    };
};

/**
 * RelationConverter
 */
class RelationConverter : public MessageConverter<
        RelationConverter,
        Relation,
        PROTO_Relation ,
        &PROTO_Relation_msg>
{
private:
    class TypeConverter: public EnumConverter<TypeConverter, Relation::Type, PROTO_Relation_Type> {
    public:
        static ProtoType encode(const LocalType& local){
            switch (local) {
                case Relation::Type::Invalid: return PROTO_Relation_Type_Invalid;
                case Relation::Type::Parent: return PROTO_Relation_Type_Parent;
                case Relation::Type::Friend: return PROTO_Relation_Type_Friend;
                case Relation::Type::Child: return PROTO_Relation_Type_Child;
            }
            return PROTO_Relation_Type_Invalid;
        };
        static LocalType decode(const ProtoType& proto){
            switch (proto) {
                case PROTO_Relation_Type_Invalid: return Relation::Type::Invalid;
                case PROTO_Relation_Type_Parent: return Relation::Type::Parent;
                case PROTO_Relation_Type_Friend: return Relation::Type::Friend;
                case PROTO_Relation_Type_Child: return Relation::Type::Child;
            }
            return Relation::Type::Invalid;
        };
    };
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
                .name = StringConverter::encoderInit(local.name),
                .type = TypeConverter::encode(local.type),
                .sinceYear = local.sinceYear,
                .comment = StringConverter::encoderInit(local.comment)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .name = StringConverter::decoderInit(local.name),
                .comment = StringConverter::decoderInit(local.comment)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        local.type = TypeConverter::decode(proto.type);
        local.sinceYear = proto.sinceYear;
        return true;
    }
};

/**
 * PersonConverter
 */
class PersonConverter : public UnionMessageConverter<
        PersonConverter,
        PersonPtr,
        PROTO_Person ,
        &PROTO_Person_msg>
{
private:
    class AdultConverter : public MessageConverter<
            AdultConverter,
            Adult,
            PROTO_Person_Adult,
            &PROTO_Person_Adult_msg>
    {
    public:
        static ProtoType encoderInit(const LocalType& local) {
            return ProtoType{
                .companyName = StringConverter::encoderInit(local.companyName),
                .position = StringConverter::encoderInit(local.position),
                .salary = local.salary
            };
        }

        static ProtoType decoderInit(LocalType& local) {
            return ProtoType{
                .companyName = StringConverter::decoderInit(local.companyName),
                .position = StringConverter::decoderInit(local.position),
            };
        }

        static bool decoderApply(const ProtoType& proto, LocalType& local){
            local.salary = proto.salary;
            return true;
        }
    };

    class ChildConverter : public MessageConverter<
            ChildConverter,
            Child,
            PROTO_Person_Child,
            &PROTO_Person_Child_msg>
    {
    private:
        using ScoresConverter = MapConverter<
                StringConverter,
                FloatConverter,
                Child::ScoresContainer,
                PROTO_Person_Child_ScoresEntry,
                &PROTO_Person_Child_ScoresEntry_msg>;
    public:
        static ProtoType encoderInit(const LocalType& local) {
            return ProtoType{
                .schoolName = StringConverter::encoderInit(local.schoolName),
                .scores = ScoresConverter::encoderCallbackInit(local.scores),
                .schoolStartYear = local.schoolStartYear
            };
        }

        static ProtoType decoderInit(LocalType& local) {
            return ProtoType{
                .schoolName = StringConverter::decoderInit(local.schoolName),
                .scores = ScoresConverter::decoderCallbackInit(local.scores)
            };
        }

        static bool decoderApply(const ProtoType& proto, LocalType& local){
            local.schoolStartYear = proto.schoolStartYear;
            return true;
        }
    };

    using LikeFoodConverter = ArrayConverter<FoodConverter, Person::FoodContainer>;
    using RelationsConverter = ArrayConverter<RelationConverter, Person::RelationsContainer>;

public:
    /**
     * Custom decoding context to decode common Person fields
     */
    struct DecoderContext {
        LocalType& local;
        std::string name;
        Person::FoodContainer likeFood;
        Person::RelationsContainer relations;

        // Constructor is needed to initialize context from the `LocalType`
        DecoderContext(LocalType &person) : local(person) {}
    };

public:
    static ProtoType encoderInit(const LocalType& local) {
        auto ret = ProtoType{
            .name = StringConverter::encoderInit(local->name),
            .age = local->age,
            .likeFood = LikeFoodConverter::encoderCallbackInit(local->likeFood),
            .relations = RelationsConverter::encoderCallbackInit(local->relations)
        };
        switch (local->getType()) {
            case Person::Type::Adult:
                ret.which_details = PROTO_Person_adult_tag;
                ret.details.adult = AdultConverter::encoderInit(*local->as<Adult>());
                break;
            case Person::Type::Child:
                ret.which_details = PROTO_Person_child_tag;
                ret.details.child = ChildConverter::encoderInit(*local->as<Child>());
                break;
        }
        return ret;
    }

    static ProtoType decoderInit(DecoderContext& ctx){
        LocalType& local = ctx.local;
        return ProtoType{
            .name = StringConverter::decoderInit(ctx.name),
            .likeFood = LikeFoodConverter::decoderCallbackInit(ctx.likeFood),
            .relations = RelationsConverter::decoderCallbackInit(ctx.relations),
            .cb_details = unionDecoderInit(local)
        };
    }

    static bool unionDecodeCallback(pb_istream_t *stream, const pb_field_t *field, LocalType &local){
        if (field->tag == PROTO_Person_adult_tag){
            auto* msg = static_cast<PROTO_Person_Adult *>(field->pData);
            local.reset(new Adult());
            *msg = AdultConverter::decoderInit(*local->as<Adult>());
        }
        else if (field->tag == PROTO_Person_child_tag){
            auto* msg = static_cast<PROTO_Person_Child *>(field->pData);
            local.reset(new Child());
            *msg = ChildConverter::decoderInit(*local->as<Child>());
        } else {
            NANOPB_CPP_ASSERT(0&&"Invalid");
            return false;
        }
        return true;
    }

    static bool decoderApply(const ProtoType& proto, DecoderContext& ctx){
        LocalType& local = ctx.local;

        local->name = std::move(ctx.name);
        local->age = proto.age;
        local->likeFood = std::move(ctx.likeFood);
        local->relations = std::move(ctx.relations);

        if (proto.which_details == PROTO_Person_adult_tag){
            AdultConverter::decoderApply(proto.details.adult, *local->as<Adult>());
        }
        else if (proto.which_details == PROTO_Person_child_tag){
            ChildConverter::decoderApply(proto.details.child, *local->as<Child>());
        } else {
            return false;
        }
        return true;
    }
};