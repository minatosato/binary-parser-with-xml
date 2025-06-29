#include "binary_parser.h"
#include "xml_struct_parser.h"
#include <cstring>
#include <stdexcept>

namespace binary_parser {

std::unique_ptr<ParsedStruct> BinaryParser::parse(
    const uint8_t* data,
    size_t data_size,
    const StructInfo& struct_info) {
    
    if (data_size < struct_info.size) {
        throw std::runtime_error("Data size is smaller than struct size");
    }
    
    auto parsed = std::make_unique<ParsedStruct>();
    parsed->struct_name = struct_info.name;
    
    for (const auto& field : struct_info.fields) {
        parsed->fields[field->name] = parseField(data, data_size, 0, *field);
    }
    
    return parsed;
}

ParsedField BinaryParser::parseField(
    const uint8_t* data,
    size_t data_size,
    size_t base_offset,
    const FieldInfo& field_info) {
    
    ParsedField parsed_field;
    parsed_field.name = field_info.name;
    
    size_t actual_offset = base_offset + field_info.offset;
    
    if (actual_offset + field_info.size > data_size) {
        throw std::runtime_error("Field offset + size exceeds data size");
    }
    
    if (field_info.type == FieldType::STRUCT || field_info.type == FieldType::UNION) {
        // Parse sub-fields
        for (const auto& sub_field : field_info.sub_fields) {
            parsed_field.sub_fields[sub_field->name] = 
                parseField(data, data_size, actual_offset, *sub_field);
        }
    } else {
        // Parse primitive value
        parsed_field.value = parseValue(data, actual_offset, field_info);
    }
    
    return parsed_field;
}

std::any BinaryParser::parseValue(
    const uint8_t* data,
    size_t offset,
    const FieldInfo& field_info) {
    
    const uint8_t* ptr = data + offset;
    
    switch (field_info.type) {
        case FieldType::UINT8:
            return static_cast<uint8_t>(*ptr);
            
        case FieldType::INT8:
            return static_cast<int8_t>(*ptr);
            
        case FieldType::UINT16: {
            uint16_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::INT16: {
            int16_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::UINT32: {
            uint32_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::INT32: {
            int32_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::UINT64: {
            uint64_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::INT64: {
            int64_t value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::FLOAT: {
            float value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        case FieldType::DOUBLE: {
            double value;
            std::memcpy(&value, ptr, sizeof(value));
            return value;
        }
        
        default:
            throw std::runtime_error("Unsupported field type");
    }
}

} // namespace binary_parser