#include "xml_struct_parser.h"
#include <tinyxml2.h>
#include <stdexcept>

namespace binary_parser {

std::unique_ptr<StructInfo> XmlStructParser::parse(const std::string& xml_file) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(xml_file.c_str());
    
    if (result != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to parse XML file: " + std::string(doc.ErrorStr()));
    }
    
    tinyxml2::XMLElement* root = doc.FirstChildElement("struct");
    if (!root) {
        throw std::runtime_error("No struct element found in XML");
    }
    
    auto struct_info = std::make_unique<StructInfo>();
    
    const char* name_attr = root->Attribute("name");
    if (name_attr) struct_info->name = name_attr;
    
    struct_info->size = root->UnsignedAttribute("size", 0);
    struct_info->packed = root->BoolAttribute("packed", false);
    
    parseSubFields(root, struct_info->fields);
    
    return struct_info;
}

void XmlStructParser::parseSubFields(const tinyxml2::XMLElement* parent, 
                                    std::vector<std::unique_ptr<FieldInfo>>& fields) {
    for (const tinyxml2::XMLElement* field_node = parent->FirstChildElement("field");
         field_node;
         field_node = field_node->NextSiblingElement("field")) {
        fields.push_back(parseField(field_node));
    }
}

std::unique_ptr<FieldInfo> XmlStructParser::parseField(const tinyxml2::XMLElement* node) {
    auto field = std::make_unique<FieldInfo>();
    
    const char* name_attr = node->Attribute("name");
    if (name_attr) field->name = name_attr;
    
    field->offset = node->UnsignedAttribute("offset", 0);
    field->size = node->UnsignedAttribute("size", 0);
    field->array_size = node->UnsignedAttribute("array_size", 1);
    field->bits = node->IntAttribute("bits", 0);
    field->bit_offset = node->IntAttribute("bit_offset", 0);
    
    // Check if it has a type attribute
    const char* type_attr = node->Attribute("type");
    if (type_attr) {
        field->type = parseFieldType(type_attr);
    } else {
        // Check for struct or union sub-elements
        const tinyxml2::XMLElement* struct_node = node->FirstChildElement("struct");
        const tinyxml2::XMLElement* union_node = node->FirstChildElement("union");
        
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