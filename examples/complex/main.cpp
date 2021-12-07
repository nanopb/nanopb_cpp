#include <stdio.h>
#include <functional>

#include "converters.hpp"

#define COMMENT(fmt, ...) printf("\n----" fmt "----\n", ##__VA_ARGS__);
#define TEST(x) {}\
    if (!(x)) { \
        printf("\033[31;1mFAILED:\033[22;39m %s:%d %s\n", __FILE__, __LINE__, #x); \
        status = 1; \
    } else { \
        printf("\033[32;1mOK:\033[22;39m %s\n", #x); \
    }

template<class CONVERTER>
int testEncodeDecode(
        const typename CONVERTER::LocalType &original,
        std::function<void(typename CONVERTER::LocalType& decoded)> callback
)
{
    int status = 0;

    using LocalType = typename CONVERTER::LocalType;

    NanoPb::StringOutputStream outputStream;

    COMMENT("Encode %s", typeid(CONVERTER).name());
    TEST(NanoPb::encode<CONVERTER>(outputStream, original));

    auto inputStream = ::NanoPb::StringInputStream(outputStream.release());

    LocalType decoded;

    COMMENT("Decode %s", typeid(CONVERTER).name());
    TEST(NanoPb::decode<CONVERTER>(inputStream, decoded));

    callback(decoded);

    return status;
}


int testAdult() {
    int status = 0;

    PersonPtr adult(new Adult(
        "Bob",
        43,
        {
            Food::Chicken,
            Food::Vegetable
            },
        {
            Relation("Alice",Relation::Type::Child, 1986, "First child"),
            Relation("John",Relation::Type::Parent, 1947, "Father")
            },
        "Microsoft",
        "Engineer",
        10250.99f
    ));

    status |= testEncodeDecode<PersonConverter>(adult,[](PersonPtr& decoded){
        NANOPB_CPP_ASSERT(decoded);

        int status = 0;

        const Adult* adult = decoded->as<Adult>();

        TEST(adult->name == "Bob");
        TEST(adult->age == 43);

        TEST(adult->likeFood.size() == 2);
        TEST(adult->likeFood.at(0) == Food::Chicken);
        TEST(adult->likeFood.at(1) == Food::Vegetable);

        TEST(adult->relations.size() == 2);

        auto& r1 = adult->relations.at(0);
        TEST(r1.name == "Alice");
        TEST(r1.type == Relation::Type::Child);
        TEST(r1.sinceYear == 1986);
        TEST(r1.comment == "First child");

        auto& r2 = adult->relations.at(1);
        TEST(r2.name == "John");
        TEST(r2.type == Relation::Type::Parent);
        TEST(r2.sinceYear == 1947);
        TEST(r2.comment == "Father");

        TEST(adult->companyName == "Microsoft");
        TEST(adult->position == "Engineer");
        TEST(adult->salary == 10250.99f);


        return status;
    });

    return status;
};

int testChild() {
    int status = 0;

    PersonPtr child(new Child(
            "Frank",
            14,
            {
                    Food::Meat,
                    Food::Chicken
            },
            {
                    Relation("Mike",Relation::Type::Friend, 2010, "Best friend"),
                    Relation("Joan",Relation::Type::Friend, 2011, "GirlFriend")
            },
            "High school",
            {
                    { "Mathematics", 9.8f},
                    { "Spanish", 6.4f}
                },
            2005
    ));


    status |= testEncodeDecode<PersonConverter>(child,[](PersonPtr& decoded){
        NANOPB_CPP_ASSERT(decoded);

        int status = 0;

        const Child* child = decoded->as<Child>();

        TEST(child->name == "Frank");
        TEST(child->age == 14);

        TEST(child->likeFood.size() == 2);
        TEST(child->likeFood.at(0) == Food::Meat);
        TEST(child->likeFood.at(1) == Food::Chicken);

        TEST(child->relations.size() == 2);

        auto& r1 = child->relations.at(0);
        TEST(r1.name == "Mike");
        TEST(r1.type == Relation::Type::Friend);
        TEST(r1.sinceYear == 2010);
        TEST(r1.comment == "Best friend");

        auto& r2 = child->relations.at(1);
        TEST(r2.name == "Joan");
        TEST(r2.type == Relation::Type::Friend);
        TEST(r2.sinceYear == 2011);
        TEST(r2.comment == "GirlFriend");


        TEST(child->schoolName == "High school");

        TEST(child->scores.size() == 2);

        TEST(child->scores.at("Mathematics") == 9.8f)
        TEST(child->scores.at("Spanish") == 6.4f)

        TEST(child->schoolStartYear == 2005);


        return status;
    });

    return status;
};


int main() {
    int status = 0;

    status |= testAdult();
    status |= testChild();

    return status;
}