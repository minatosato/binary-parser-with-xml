#include "binary_parser.h"
#include "xml_struct_parser.h"
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <climits>

namespace binary_parser {

// Check system endianness
static bool isLittleEndian() {
    uint16_t test = 1;
    return *reinterpret_cast<uint8_t*>(&test) == 1;
}

bool BinaryParser::needsByteSwap() const {
    bool systemIsLittleEndian = isLittleEndian();
    return (endianness_ == Endianness::LITTLE) != systemIsLittleEndian;
}

uint16_t BinaryParser::byteSwap16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

uint32_t BinaryParser::byteSwap32(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

uint64_t BinaryParser::byteSwap64(uint64_t value) {
    return ((value >> 56) & 0xFFULL) |
           ((value >> 40) & 0xFF00ULL) |
           ((value >> 24) & 0xFF0000ULL) |
           ((value >> 8) & 0xFF000000ULL) |
           ((value << 8) & 0xFF00000000ULL) |
           ((value << 24) & 0xFF0000000000ULL) |
           ((value << 40) & 0xFF000000000000ULL) |
           ((value << 56) & 0xFF00000000000000ULL);
}

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
        std::string error_msg = "Field offset + size exceeds data size: " +
                                field_info.name + " at offset " + std::to_string(actual_offset) +
                                " with size " + std::to_string(field_info.size) +
                                " exceeds data size " + std::to_string(data_size);
        throw std::runtime_error(error_msg);
    }
    
    if (field_info.type == FieldType::STRUCT || field_info.type == FieldType::UNION) {
        // Parse sub-fields
        for (const auto& sub_field : field_info.sub_fields) {
            parsed_field.sub_fields[sub_field->name] = 
                parseField(data, data_size, actual_offset, *sub_field);
        }
    } else if (field_info.array_size > 1) {
        // Parse array
        parsed_field.value = parseArray(data, data_size, actual_offset, field_info);
    } else if (field_info.bits > 0) {
        // Parse bitfield
        parsed_field.value = parseBitfield(data, actual_offset, field_info);
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
            if (needsByteSwap()) value = byteSwap16(value);
            return value;
        }
        
        case FieldType::INT16: {
            int16_t value;
            std::memcpy(&value, ptr, sizeof(value));
            if (needsByteSwap()) value = byteSwap16(value);
            return value;
        }
        
        case FieldType::UINT32: {
            uint32_t value;
            std::memcpy(&value, ptr, sizeof(value));
            if (needsByteSwap()) value = byteSwap32(value);
            return value;
        }
        
        case FieldType::INT32: {
            int32_t value;
            std::memcpy(&value, ptr, sizeof(value));
            if (needsByteSwap()) value = byteSwap32(value);
            return value;
        }
        
        case FieldType::UINT64: {
            uint64_t value;
            std::memcpy(&value, ptr, sizeof(value));
            if (needsByteSwap()) value = byteSwap64(value);
            return value;
        }
        
        case FieldType::INT64: {
            int64_t value;
            std::memcpy(&value, ptr, sizeof(value));
            if (needsByteSwap()) value = byteSwap64(value);
            return value;
        }
        
        case FieldType::FLOAT: {
            float value;
            if (needsByteSwap()) {
                uint32_t temp;
                std::memcpy(&temp, ptr, sizeof(temp));
                temp = byteSwap32(temp);
                std::memcpy(&value, &temp, sizeof(value));
            } else {
                std::memcpy(&value, ptr, sizeof(value));
            }
            return value;
        }
        
        case FieldType::DOUBLE: {
            double value;
            if (needsByteSwap()) {
                uint64_t temp;
                std::memcpy(&temp, ptr, sizeof(temp));
                temp = byteSwap64(temp);
                std::memcpy(&value, &temp, sizeof(value));
            } else {
                std::memcpy(&value, ptr, sizeof(value));
            }
            return value;
        }
        
        default:
            throw std::runtime_error("Unsupported field type");
    }
}

std::any BinaryParser::parseArray(
    const uint8_t* data,
    size_t data_size,
    size_t offset,
    const FieldInfo& field_info) {
    
    size_t element_size = field_info.size / field_info.array_size;
    
    switch (field_info.type) {
        case FieldType::UINT8: {
            std::vector<uint8_t> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                array.push_back(data[offset + i]);
            }
            return array;
        }
        
        case FieldType::UINT16: {
            std::vector<uint16_t> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                uint16_t value;
                std::memcpy(&value, data + offset + i * element_size, sizeof(value));
                if (needsByteSwap()) value = byteSwap16(value);
                array.push_back(value);
            }
            return array;
        }
        
        case FieldType::UINT32: {
            std::vector<uint32_t> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                uint32_t value;
                std::memcpy(&value, data + offset + i * element_size, sizeof(value));
                if (needsByteSwap()) value = byteSwap32(value);
                array.push_back(value);
            }
            return array;
        }
        
        case FieldType::UINT64: {
            std::vector<uint64_t> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                uint64_t value;
                std::memcpy(&value, data + offset + i * element_size, sizeof(value));
                if (needsByteSwap()) value = byteSwap64(value);
                array.push_back(value);
            }
            return array;
        }
        
        case FieldType::FLOAT: {
            std::vector<float> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                float value;
                if (needsByteSwap()) {
                    uint32_t temp;
                    std::memcpy(&temp, data + offset + i * element_size, sizeof(temp));
                    temp = byteSwap32(temp);
                    std::memcpy(&value, &temp, sizeof(value));
                } else {
                    std::memcpy(&value, data + offset + i * element_size, sizeof(value));
                }
                array.push_back(value);
            }
            return array;
        }
        
        case FieldType::DOUBLE: {
            std::vector<double> array;
            array.reserve(field_info.array_size);
            for (size_t i = 0; i < field_info.array_size; i++) {
                double value;
                if (needsByteSwap()) {
                    uint64_t temp;
                    std::memcpy(&temp, data + offset + i * element_size, sizeof(temp));
                    temp = byteSwap64(temp);
                    std::memcpy(&value, &temp, sizeof(value));
                } else {
                    std::memcpy(&value, data + offset + i * element_size, sizeof(value));
                }
                array.push_back(value);
            }
            return array;
        }
        
        case FieldType::UNKNOWN:
        case FieldType::STRUCT:
        case FieldType::UNION: {
            // For unknown types (typedefs), structs, and unions, parse as array of ParsedFields
            std::vector<std::any> array;
            array.reserve(field_info.array_size);
            
            // Each element should have sub_fields
            if (!field_info.sub_fields.empty()) {
                for (size_t i = 0; i < field_info.array_size; i++) {
                    ParsedField element;
                    element.name = std::to_string(i);
                    for (const auto& sub_field : field_info.sub_fields) {
                        element.sub_fields[sub_field->name] = 
                            parseField(data, data_size, offset + i * element_size, *sub_field);
                    }
                    array.push_back(element);
                }
            } else {
                // If no sub_fields, just skip this array
                for (size_t i = 0; i < field_info.array_size; i++) {
                    array.push_back(std::any{});  // Empty element
                }
            }
            return array;
        }
        
        default:
            throw std::runtime_error("Unsupported array element type");
    }
}

std::any BinaryParser::parseBitfield(
    const uint8_t* data,
    size_t offset,
    const FieldInfo& field_info) {
    
    // Read the full value from memory
    const uint8_t* ptr = data + offset;
    uint64_t full_value = 0;
    
    // Read based on the field size
    switch (field_info.size) {
        case 1:
            full_value = *ptr;
            break;
        case 2: {
            uint16_t temp;
            std::memcpy(&temp, ptr, sizeof(temp));
            if (needsByteSwap()) temp = byteSwap16(temp);
            full_value = temp;
            break;
        }
        case 4: {
            uint32_t temp;
            std::memcpy(&temp, ptr, sizeof(temp));
            if (needsByteSwap()) temp = byteSwap32(temp);
            full_value = temp;
            break;
        }
        case 8:
            std::memcpy(&full_value, ptr, sizeof(full_value));
            if (needsByteSwap()) full_value = byteSwap64(full_value);
            break;
        default:
            throw std::runtime_error("Unsupported bitfield size");
    }
    
    // Extract the bitfield value
    // Create mask with the specified number of bits
    uint64_t mask = (1ULL << field_info.bits) - 1;
    
    // Shift right to get the bits we want, then apply mask
    uint64_t bitfield_value = (full_value >> field_info.bit_offset) & mask;
    
    // Return the value as the appropriate type
    switch (field_info.type) {
        case FieldType::UINT8:
            return static_cast<uint8_t>(bitfield_value);
        case FieldType::UINT16:
            return static_cast<uint16_t>(bitfield_value);
        case FieldType::UINT32:
            return static_cast<uint32_t>(bitfield_value);
        case FieldType::UINT64:
            return bitfield_value;
        case FieldType::INT8:
            // Sign extend if needed
            if (field_info.bits < 8 && (bitfield_value & (1ULL << (field_info.bits - 1)))) {
                bitfield_value |= ~((1ULL << field_info.bits) - 1);
            }
            return static_cast<int8_t>(bitfield_value);
        case FieldType::INT16:
            if (field_info.bits < 16 && (bitfield_value & (1ULL << (field_info.bits - 1)))) {
                bitfield_value |= ~((1ULL << field_info.bits) - 1);
            }
            return static_cast<int16_t>(bitfield_value);
        case FieldType::INT32:
            if (field_info.bits < 32 && (bitfield_value & (1ULL << (field_info.bits - 1)))) {
                bitfield_value |= ~((1ULL << field_info.bits) - 1);
            }
            return static_cast<int32_t>(bitfield_value);
        case FieldType::INT64:
            if (field_info.bits < 64 && (bitfield_value & (1ULL << (field_info.bits - 1)))) {
                bitfield_value |= ~((1ULL << field_info.bits) - 1);
            }
            return static_cast<int64_t>(bitfield_value);
        default:
            throw std::runtime_error("Unsupported bitfield type");
    }
}

} // namespace binary_parser