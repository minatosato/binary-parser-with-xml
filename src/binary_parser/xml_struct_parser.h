#ifndef XML_STRUCT_PARSER_H
#define XML_STRUCT_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace tinyxml2 {
    class XMLElement;
}

namespace binary_parser {

enum class FieldType {
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    UINT64,
    INT64,
    FLOAT,
    DOUBLE,
    CHAR,
    STRUCT,
    UNION,
    UNKNOWN
};

struct FieldInfo {
    std::string name;
    FieldType type;
    size_t offset;
    size_t size;
    size_t array_size = 1;  // 1 if not an array
    int bits = 0;  // 0 if not a bitfield
    int bit_offset = 0;  // bit offset within the field
    
    // For struct/union fields
    std::vector<std::unique_ptr<FieldInfo>> sub_fields;
    bool is_union = false;
};

struct StructInfo {
    std::string name;
    size_t size;
    bool packed = false;
    std::vector<std::unique_ptr<FieldInfo>> fields;
};

class XmlStructParser {
public:
    XmlStructParser() = default;
    
    std::unique_ptr<StructInfo> parse(const std::string& xml_file);
    
private:
    std::unique_ptr<FieldInfo> parseField(const tinyxml2::XMLElement* node);
    FieldType parseFieldType(const std::string& type_str);
    void parseSubFields(const tinyxml2::XMLElement* parent, std::vector<std::unique_ptr<FieldInfo>>& fields);
};

} // namespace binary_parser

#endif // XML_STRUCT_PARSER_H