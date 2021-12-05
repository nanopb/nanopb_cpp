#pragma once

using namespace NanoPb::Converter;

struct InnerMessage {
    enum class Type {
        UnionInnerOne,
        UnionInnerTwo,
        UnionInnerThree
    };
    virtual ~InnerMessage(){}
    virtual Type getType() const = 0;

    template<class T>
    T* as(){ return static_cast<T*>(this); }
    template<class T>
    const T* as() const { return static_cast<const T*>(this); }

    bool operator==(const InnerMessage &rhs) const {
        return rhs.getType() == getType();
    }

    bool operator!=(const InnerMessage &rhs) const {
        return !(rhs == *this);
    }
};

struct UnionInnerOne : public InnerMessage {
    int number = 0;

    UnionInnerOne() = default;
    UnionInnerOne(const UnionInnerOne&) = delete;
    UnionInnerOne(UnionInnerOne&&) = default;

    UnionInnerOne(int number) : number(number) {}

    Type getType() const override { return Type::UnionInnerOne; }

    bool operator==(const UnionInnerOne &rhs) const {
        return static_cast<const InnerMessage &>(*this) == static_cast<const InnerMessage &>(rhs) &&
               number == rhs.number;
    }

    bool operator!=(const UnionInnerOne &rhs) const {
        return !(rhs == *this);
    }
};

struct UnionInnerTwo : public InnerMessage {
    std::string str;

    UnionInnerTwo() = default;
    UnionInnerTwo(const UnionInnerTwo&) = delete;
    UnionInnerTwo(UnionInnerTwo&&) = default;

    UnionInnerTwo(const std::string &str) : str(str) {}

    Type getType() const override { return Type::UnionInnerTwo; }

    bool operator==(const UnionInnerTwo &rhs) const {
        return static_cast<const InnerMessage &>(*this) == static_cast<const InnerMessage &>(rhs) &&
               str == rhs.str;
    }

    bool operator!=(const UnionInnerTwo &rhs) const {
        return !(rhs == *this);
    }
};

struct UnionInnerThree : public InnerMessage {
    using ValuesContainer = std::vector<uint32_t>;
    ValuesContainer values;

    UnionInnerThree() = default;
    UnionInnerThree(const UnionInnerThree&) = delete;
    UnionInnerThree(UnionInnerThree&&) = default;

    UnionInnerThree(const ValuesContainer &values) : values(values) {}

    Type getType() const override { return Type::UnionInnerThree; }

    bool operator==(const UnionInnerThree &rhs) const {
        return static_cast<const InnerMessage &>(*this) == static_cast<const InnerMessage &>(rhs) &&
               values == rhs.values;
    }

    bool operator!=(const UnionInnerThree &rhs) const {
        return !(rhs == *this);
    }
};

/******************************************************************************************/

class UnionInnerOneConverter : public MessageConverter<
        UnionInnerOneConverter,
        UnionInnerOne,
        PROTO_UnionInnerOne,
        &PROTO_UnionInnerOne_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType {
                .number = local.number
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        local.number = proto.number;
        return true;
    }
};

class UnionInnerTwoConverter : public MessageConverter<
        UnionInnerTwoConverter,
        UnionInnerTwo,
        PROTO_UnionInnerTwo,
        &PROTO_UnionInnerTwo_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType {
                .str = StringConverter::encoderCallbackInit(local.str)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .str = StringConverter::decoderCallbackInit(local.str)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};

class UnionInnerThreeConverter : public MessageConverter<
        UnionInnerThreeConverter,
        UnionInnerThree,
        PROTO_UnionInnerThree,
        &PROTO_UnionInnerThree_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType {
                .values = ArrayConverter<UInt32Converter,UnionInnerThree::ValuesContainer>::encoderCallbackInit(local.values)
        };
    }

    static ProtoType decoderInit(LocalType& local){
        return ProtoType{
                .values = ArrayConverter<UInt32Converter, UnionInnerThree::ValuesContainer>::decoderCallbackInit(local.values)
        };
    }

    static bool decoderApply(const ProtoType& proto, LocalType& local){
        return true;
    }
};