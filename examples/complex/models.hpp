#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <list>

namespace Model {
    enum class Food {
        Invalid,
        Meat,
        Chicken,
        Vegetable
    };

    struct Relation {
        enum class Type {
            Invalid,
            Parent,
            Friend,
            Child
        };
        std::string name;
        Type type = Type::Invalid;
        uint32_t sinceYear = 0;
        std::string comment;

        Relation() = default;
        Relation(const Relation&) = default;

        Relation(const std::string& name, Type type, uint32_t sinceYear, const std::string &comment) :
            name(name), type(type), sinceYear(sinceYear), comment(comment)
        {}
    };

    struct Person {
        enum class Type {
            Adult,
            Child
        };

        using FoodContainer = std::vector<Food>;
        using RelationsContainer = std::vector<Relation>;

        std::string name;
        unsigned age = 0;
        FoodContainer likeFood;
        RelationsContainer relations;

        Person() = default;
        Person(const Person&) = default;
        Person(const std::string &name, unsigned int age,
               const FoodContainer &likeFood, const RelationsContainer &relations)
               : name(name), age(age), likeFood(likeFood), relations(relations)
        {}

        virtual ~Person() = default;
        virtual Type getType() const = 0;

        template<class T> T* as(){ return static_cast<T*>(this); }
        template<class T> const T* as() const { return static_cast<const T*>(this); }
    };

    struct Adult : public Person {
        std::string companyName;
        std::string position;
        float salary = 0;

        Adult() = default;
        Adult(const Adult&) = default;

        Adult(const std::string &name, unsigned int age, const FoodContainer &likeFood,
              const RelationsContainer &relations, const std::string &companyName,
              const std::string &position, float salary) :
              Person(name, age, likeFood, relations), companyName(companyName), position(position), salary(salary)
        {}

        Type getType() const override { return Type::Adult; }
    };

    struct Child : public Person {
        using ScoresContainer = std::map<std::string, float>;

        std::string schoolName;
        std::map<std::string, float> scores;
        uint32_t schoolStartYear = 0;

        Child() = default;

        Child(const std::string &name, unsigned int age, const FoodContainer &likeFood,
              const RelationsContainer &relations, const std::string &schoolName,
              const std::map<std::string, float> &scores, uint32_t schoolStartYear) :
              Person(name, age, likeFood, relations), schoolName(schoolName),
              scores(scores), schoolStartYear(schoolStartYear)
          {}

        Type getType() const override { return Type::Child; }
    };

    using PersonPtr = std::unique_ptr<Person>;

}