#ifndef JSON_CONVERTER_H
#define JSON_CONVERTER_H

#include "binary_parser.h"
#include "../json/json_value.h"
#include <any>

namespace binary_parser {

struct JsonConvertOptions {
    bool compact = true;           // Compact output (no pretty print)
    bool include_type_info = false;  // Include type information in output
};

class JsonConverter {
public:
    JsonConverter() = default;
    
    // Convert ParsedStruct to JsonValue
    JsonValue convert(const ParsedStruct& parsed_struct, 
                     const JsonConvertOptions& options = JsonConvertOptions());
    
private:
    // Convert ParsedField to JsonValue
    JsonValue convertField(const ParsedField& field, const JsonConvertOptions& options);
    
    // Convert std::any value to JsonValue
    JsonValue convertValue(const std::any& value);
    
    // Get type name from std::any
    std::string getTypeName(const std::any& value);
    
    // Check if vector contains char/uint8_t that represents a string
    bool isCharArray(const std::vector<uint8_t>& vec);
    
    // Convert char array to string
    std::string charArrayToString(const std::vector<uint8_t>& vec);
};

} // namespace binary_parser

#endif // JSON_CONVERTER_H