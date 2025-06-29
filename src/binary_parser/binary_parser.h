#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <any>

namespace binary_parser {

enum class Endianness {
    LITTLE,  // Default
    BIG
};

struct StructInfo;
struct FieldInfo;

struct ParsedField {
    std::string name;
    std::any value;
    std::unordered_map<std::string, ParsedField> sub_fields;
};

struct ParsedStruct {
    std::string struct_name;
    std::unordered_map<std::string, ParsedField> fields;
};

class BinaryParser {
public:
    BinaryParser(Endianness endianness = Endianness::LITTLE) 
        : endianness_(endianness) {}
    
    // Parse binary data using struct definition from XML
    std::unique_ptr<ParsedStruct> parse(
        const uint8_t* data,
        size_t data_size,
        const StructInfo& struct_info
    );
    
    // Get parsed value as specific type
    template<typename T>
    static T getValue(const ParsedField& field) {
        return std::any_cast<T>(field.value);
    }
    
    // Get array values
    template<typename T>
    static std::vector<T> getArray(const ParsedField& field) {
        return std::any_cast<std::vector<T>>(field.value);
    }
    
private:
    ParsedField parseField(
        const uint8_t* data,
        size_t data_size,
        size_t base_offset,
        const FieldInfo& field_info
    );
    
    std::any parseValue(
        const uint8_t* data,
        size_t offset,
        const FieldInfo& field_info
    );
    
    std::any parseArray(
        const uint8_t* data,
        size_t offset,
        const FieldInfo& field_info
    );
    
    std::any parseBitfield(
        const uint8_t* data,
        size_t offset,
        const FieldInfo& field_info
    );
    
    // Byte swapping utilities
    uint16_t byteSwap16(uint16_t value);
    uint32_t byteSwap32(uint32_t value);
    uint64_t byteSwap64(uint64_t value);
    
    // Check if byte swapping is needed
    bool needsByteSwap() const;
    
    Endianness endianness_;
};

} // namespace binary_parser

#endif // BINARY_PARSER_H