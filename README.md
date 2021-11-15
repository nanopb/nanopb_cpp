# NanoPb C++ 

Extends C++ support for the [NanoPb](https://github.com/nanopb/nanopb).

## Features

* One-line call encode/decode for complicated messages.
* Support for custom union inner message encode/decode with some [limitations](#limitations).

## Supported types:

* `NanoPb::AbstractScalarConverter` - Basic scalar type like `enum`.
* `NanoPb::AbstractMessageConverter` - Converter for messages.
* `NanoPb::AbstractCallbackConverter` - Converter for callbacks.
* `NanoPb::StringConverter` - Converter for variable size string, mapped to `std::string`.
* `NanoPb::ArrayUnsignedConverter` - Repeated convertor for `std::vector<unsigned>` or `std::list<unsigned>` (Also support other signed types: `uint64_t`, `uint32_t`, etc).
* `NanoPb::ArraySignedConverter` - Repeated convertor for `std::vector<int>` or `std::list<int>` (Also support other signed types: `int64_t`, `int32_t`, etc).
* `NanoPb::ArrayStringConverter` - Repeated convertor for `std::vector<std::string>` or `std::list<std::string>`.
* `NanoPb::ArrayMessageConverter` - Repeated convertor for `std::vector<CLASS>` or `std::list<CLASS>`, NOTE: `AbstractMessageConverter` for  `CLASS` should be defined. 
* `NanoPb::MapConverter` - Map with any type of the key and value.

## Usage

* Define C++ `class/struct` for each protobuf message.
* Define converter for each local/protobuf message pair, inherit from `NanoPb::AbstractMessageConverter`.
* Define callback converters for callback field, inherit from `NanoPb::AbstractCallbackConverter`.

See [tests](test/tests) folder for examples.


## Limitations

* All C++ classes should have default constructor 
* All C++ classes should have copy constructor or move constructor
* Union messages are supported ony with `no_unions:true` option (see [union_message.options](test/common/union_message.options)) for the example.