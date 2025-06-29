#include "xml_struct_parser.h"
#include <pugixml.hpp>
#include <stdexcept>

namespace binary_parser {

std::unique_ptr<StructInfo> XmlStructParser::parse(const std::string& xml_file) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(xml_file.c_str());
    
    if (!result) {
        throw std::runtime_error("Failed to parse XML file: " + std::string(result.description()));
    }
    
    pugi::xml_node root = doc.child("struct");
    if (!root) {
        throw std::runtime_error("No struct element found in XML");
    }
    
    auto struct_info = std::make_unique<StructInfo>();
    struct_info->name = root.attribute("name").as_string();
    struct_info->size = root.attribute("size").as_uint();
    struct_info->packed = root.attribute("packed").as_bool(false);
    
    parseSubFields(root, struct_info->fields);
    
    return struct_info;
}

void XmlStructParser::parseSubFields(const pugi::xml_node& parent, 
                                    std::vector<std::unique_ptr<FieldInfo>>& fields) {
    for (pugi::xml_node field_node : parent.children("field")) {
        fields.push_back(parseField(field_node));
    }
}

std::unique_ptr<FieldInfo> XmlStructParser::parseField(const pugi::xml_node& node) {
    auto field = std::make_unique<FieldInfo>();
    
    field->name = node.attribute("name").as_string();
    field->offset = node.attribute("offset").as_uint();
    field->size = node.attribute("size").as_uint();
    field->array_size = node.attribute("array_size").as_uint(1);
    field->bits = node.attribute("bits").as_int(0);
    
    // Check if it has a type attribute
    if (node.attribute("type")) {
        field->type = parseFieldType(node.attribute("type").as_string());
    } else {
        // Check for struct or union sub-elements
        pugi::xml_node struct_node = node.child("struct");
        pugi::xml_node union_node = node.child("union");
        
        if (struct_node) {
            field->type = FieldType::STRUCT;
            field->is_union = false;
            parseSubFields(struct_node, field->sub_fields);
        } else if (union_node) {
            field->type = FieldType::UNION;
            field->is_union = true;
            parseSubFields(union_node, field->sub_fields);
        } else {
            field->type = FieldType::UNKNOWN;
        }
    }
    
    return field;
}

FieldType XmlStructParser::parseFieldType(const std::string& type_str) {
    if (type_str == "uint8_t") return FieldType::UINT8;
    if (type_str == "int8_t") return FieldType::INT8;
    if (type_str == "uint16_t") return FieldType::UINT16;
    if (type_str == "int16_t") return FieldType::INT16;
    if (type_str == "uint32_t") return FieldType::UINT32;
    if (type_str == "int32_t") return FieldType::INT32;
    if (type_str == "uint64_t") return FieldType::UINT64;
    if (type_str == "int64_t") return FieldType::INT64;
    if (type_str == "float") return FieldType::FLOAT;
    if (type_str == "double") return FieldType::DOUBLE;
    if (type_str == "char") return FieldType::UINT8;  // Treat char as uint8_t
    
    return FieldType::UNKNOWN;
}

} // namespace binary_parser