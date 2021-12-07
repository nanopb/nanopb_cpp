# WARNING: project is not production ready until it will be in `main` branch.

# NanoPb C++ 

![ci](https://github.com/hacker-cb/nanopb_cpp/actions/workflows/ci.yaml/badge.svg)

Extends C++ support for the [nanopb] and [protobuf].

Project is designed to map structures generated by [nanopb] and C++ models for easy encoding/decode complicated types like variable-length strings, arrays, maps, oneof(union) messages, etc.

## Features

* Written to support C++ 11 standard.
* One-line call encode/decode for complicated messages.
* Reusable converters for inner messages.
* Template-based classes with static methods for converters.

## Supported scalar types:

All scalar types are defined in `NanoPb::Type` namespace.

**Base scalar types:**

* Int32
* SInt32
* UInt32
* Fixed32
* SFixed32
* Float
* Bool
* String
* Bytes

**64 bit scalar types, supported only if definition `PB_WITHOUT_64BIT` was not set:**

* Int64
* SInt64
* UInt64
* Fixed64
* SFixed64
* Double

## Converters

### Base converter classes:

* #### `CallbackConverter` - Converter for callback fields.
  TODO: describe

* #### `EnumConverter` - Converter for `enum`
  TODO: describe

* #### `MessageConverter` - Converter for message.
  TODO: describe


### Helper converters: 

* `UnionMessageConverter` - Converter for union (oneof) messages. Derived from `MessageConverter`.

### Scalar converters:

All scalar converters are derived from `CallbackConverter` class.

**Base scalar converters:**

* `Int32Converter`
* `SInt32Converter`
* `UInt32Converter`
* `Fixed32Converter`
* `SFixed32Converter`
* `FloatConverter`
* `BoolConverter`
* `StringConverter`
* `BytesConverter`

**64 bit scalar types, supported only if definition `PB_WITHOUT_64BIT` was not set:**

* `Int64Converter`
* `SInt64Converter`
* `UInt64Converter`
* `Fixed64Converter`
* `SFixed64Converter`
* `DoubleConverter`

### Container converters:

All container converters are derived from `CallbackConverter` class.

* `ArrayConverter` - Array converter for `std::vector<xxx>` or `std::list<xxx>`. 
* `MapConverter` - Map with any type of the key and value.

## Install

### CMake install

* Use `add subdirectory()` or `CPMAddPackage()` from [CPM] to add **nanopb_cpp** to your CMake project.   
* Set `NANOPB_VERSION` cmake variable to use custom nanopb version/git tag.
* [nanopb] will be downloaded via [CPM]. Set `NANOPB_ROOT` cmake variable to use your own nanopb location if you want to skip download.
* Use `target_link_libraries(YOUR_TARGET nanopb_cpp)` to add dependency.

[CPM] example:
```cmake
set(NANOPB_CPP_VERSION master)
CPMAddPackage(NAME lib_nanopb_cpp GITHUB_REPOSITORY hacker-cb/nanopb_cpp GIT_TAG ${NANOPB_CPP_VERSION})
```
NOTE: You can add `DOWNLOAD_ONLY YES` to `CPMAddPackage()` to prevent calling `add_subdirectory()`. Project will be just downloaded in this case and you should add sources to your target manually.  

### Manual install

* Install [nanopb]. Generate code from it.
* Add [nanopb_cpp.cpp](nanopb_cpp.cpp) to your project sources.

### PlatformIO install

FIXME: TODO.

## Usage

* Use `#include nanopb_cpp.h` in your code.
* Define C++ `class/struct` for each protobuf message generated by [nanopb].
* Define converter for each local/protobuf structs, inherit from `MessageConverter`.
* Define callback converters for callback fields, inherit from `CallbackConverter`.
* Use other converters depending on your protobuf structures.

## Limitations

* All C++ classes should have default constructor
* All C++ classes should have copy constructor or move constructor

## Examples

* See [examples](examples) folder for the examples.
* See [tests](test/tests) folder. (NOTE: copy constructors are disabled just for test reasons. You don't need to disable them actually.) 

### Basic string example:

#### Protobuf:

```protobuf
syntax = "proto3";

package PROTO;

message TestMessage {
  string str = 1;
}
```

#### C++ local model:

```c++
struct TestMessage {
    std::string str;

    TestMessage() = default;
    TestMessage(const TestMessage&) = delete;
    TestMessage(TestMessage&&) = default;
};
```

#### Converter:

```c++
using namespace NanoPb::Converter;

class TestMessageConverter : public MessageConverter<
        TestMessageConverter,
        TestMessage,
        PROTO_TestMessage,
        &PROTO_TestMessage_msg>
{
public:
    static ProtoType encoderInit(const LocalType& local) {
        return ProtoType{
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
```

#### Usage:

```c++
const TestMessage original(
        {"My super string"}
        );

// Define output stream. 
NanoPb::StringOutputStream outputStream;

// Encode
if (!NanoPb::encode<TestMessageConverter>(outputStream, original)){
    // encode error
}

// Define input stream
auto inputStream = NanoPb::StringInputStream(outputStream.release());

TestMessage decoded;

/// Decode message
if (!NanoPb::decode<TestMessageConverter>(inputStream, decoded)){
    // decode error
}
```

## Limitations

* ArrayConverter can't work with `std::vector<bool>`, because it has specific implementation which is not compatible with standard behavior.

[protobuf]: https://developers.google.com/protocol-buffers
[nanopb]: https://github.com/nanopb/nanopb
[CPM]: https://github.com/cpm-cmake/CPM.cmake
