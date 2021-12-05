#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace Model {
    enum class Food {
        Invalid,
        Meat,
        Chicken,
        Vegetable
    };

    enum class RelationType {
        Invalid,
        Parent,
        Friend,
        Child
    };

    struct Person {
        enum class Type {
            Adult,
            Child
        };

        std::string name;
        unsigned age = 0;
        std::vector<Food> likeFood;
        std::map<std::string, RelationType> relations;

        virtual ~Person() = delete;
        virtual Type getType() const = 0;

        template<class T> T* as(){ return static_cast<T*>(this); }
        template<class T> const T* as() const { return static_cast<const T*>(this); }
    };

    struct Adult : public Person {
        std::string companyName;
        std::string position;
        float salary;
        Type getType() const override { return Type::Adult; }
    };

    struct Child : public Person {
        std::string schoolName;
        std::vector<float> lastScores;
        Type getType() const override { return Type::Child; }
    };

}