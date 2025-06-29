#include "json_converter.h"
#include <typeinfo>
#include <vector>
#include <cstring>

namespace binary_parser {

JsonValue JsonConverter::convert(const ParsedStruct& parsed_struct, 
                                const JsonConvertOptions& options) {
    // For simple output, just return the fields directly
    JsonValue result = JsonValue::createObject();
    
    for (const auto& [field_name, field] : parsed_struct.fields) {
        result.set(field_name, convertFieldSimple(field));
    }
    
    return result;
}

JsonValue JsonConverter::convertField(const ParsedField& field, 
                                     const JsonConvertOptions& options) {
    JsonValue field_obj;
    
    field_obj.set("name", JsonValue(field.name));
    
    // If it has sub_fields, it's a struct or union
    if (!field.sub_fields.empty()) {
        JsonValue sub_fields_obj;
        for (const auto& [sub_name, sub_field] : field.sub_fields) {
            sub_fields_obj.set(sub_name, convertField(sub_field, options));
        }
        field_obj.set("sub_fields", sub_fields_obj);
    } else {
        // Convert the value
        field_obj.set("value", convertValue(field.value));
    }
    
    // Add type information if requested
    if (options.include_type_info && field.value.has_value()) {
        field_obj.set("type", JsonValue(getTypeName(field.value)));
    }
    
    return field_obj;
}

JsonValue JsonConverter::convertFieldSimple(const ParsedField& field) {
    // If it has sub_fields, it's a struct or union
    if (!field.sub_fields.empty()) {
        JsonValue obj = JsonValue::createObject();
        for (const auto& [sub_name, sub_field] : field.sub_fields) {
            obj.set(sub_name, convertFieldSimple(sub_field));
        }
        return obj;
    } else {
        // Just return the value directly
        return convertValue(field.value);
    }
}

JsonValue JsonConverter::convertValue(const std::any& value) {
    if (!value.has_value()) {
        return JsonValue();  // null
    }
    
    // Try to convert each possible type
    try {
        // Boolean types
        if (value.type() == typeid(bool)) {
            return JsonValue(std::any_cast<bool>(value));
        }
        
        // Integer types
        if (value.type() == typeid(uint8_t)) {
            return JsonValue(static_cast<int>(std::any_cast<uint8_t>(value)));
        }
        if (value.type() == typeid(int8_t)) {
            return JsonValue(static_cast<int>(std::any_cast<int8_t>(value)));
        }
        if (value.type() == typeid(uint16_t)) {
            return JsonValue(static_cast<int>(std::any_cast<uint16_t>(value)));
        }
        if (value.type() == typeid(int16_t)) {
            return JsonValue(static_cast<int>(std::any_cast<int16_t>(value)));
        }
        if (value.type() == typeid(uint32_t)) {
            return JsonValue(static_cast<double>(std::any_cast<uint32_t>(value)));
        }
        if (value.type() == typeid(int32_t)) {
            return JsonValue(static_cast<double>(std::any_cast<int32_t>(value)));
        }
        if (value.type() == typeid(uint64_t)) {
            return JsonValue(static_cast<double>(std::any_cast<uint64_t>(value)));
        }
        if (value.type() == typeid(int64_t)) {
            return JsonValue(static_cast<double>(std::any_cast<int64_t>(value)));
        }
        
        // Floating point types
        if (value.type() == typeid(float)) {
            return JsonValue(static_cast<double>(std::any_cast<float>(value)));
        }
        if (value.type() == typeid(double)) {
            return JsonValue(std::any_cast<double>(value));
        }
        
        // Array types
        if (value.type() == typeid(std::vector<uint8_t>)) {
            auto vec = std::any_cast<std::vector<uint8_t>>(value);
            
            // Check if it's a char array (string)
            if (isCharArray(vec)) {
                return JsonValue(charArrayToString(vec));
            }
            
            // Otherwise, convert as array
            JsonValue arr = JsonValue::createArray();
            for (uint8_t val : vec) {
                arr.pushBack(JsonValue(static_cast<int>(val)));
            }
            return arr;
        }
        
        if (value.type() == typeid(std::vector<uint16_t>)) {
            auto vec = std::any_cast<std::vector<uint16_t>>(value);
            JsonValue arr;
            for (uint16_t val : vec) {
                arr.pushBack(JsonValue(static_cast<int>(val)));
            }
            return arr;
        }
        
        if (value.type() == typeid(std::vector<uint32_t>)) {
            auto vec = std::any_cast<std::vector<uint32_t>>(value);
            JsonValue arr = JsonValue::createArray();
            for (uint32_t val : vec) {
                arr.pushBack(JsonValue(static_cast<double>(val)));
            }
            return arr;
        }
        
        if (value.type() == typeid(std::vector<uint64_t>)) {
            auto vec = std::any_cast<std::vector<uint64_t>>(value);
            JsonValue arr = JsonValue::createArray();
            for (uint64_t val : vec) {
                arr.pushBack(JsonValue(static_cast<double>(val)));
            }
            return arr;
        }
        
        if (value.type() == typeid(std::vector<float>)) {
            auto vec = std::any_cast<std::vector<float>>(value);
            JsonValue arr = JsonValue::createArray();
            for (float val : vec) {
                arr.pushBack(JsonValue(static_cast<double>(val)));
            }
            return arr;
        }
        
        if (value.type() == typeid(std::vector<double>)) {
            auto vec = std::any_cast<std::vector<double>>(value);
            JsonValue arr = JsonValue::createArray();
            for (double val : vec) {
                arr.pushBack(JsonValue(val));
            }
            return arr;
        }
        
        // Handle ParsedField arrays (for typedef arrays)
        if (value.type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(value);
            JsonValue arr = JsonValue::createArray();
            for (const auto& elem : vec) {
                if (elem.type() == typeid(ParsedField)) {
                    arr.pushBack(convertFieldSimple(std::any_cast<ParsedField>(elem)));
                } else {
                    arr.pushBack(convertValue(elem));
                }
            }
            return arr;
        }
        
    } catch (const std::bad_any_cast& e) {
        // If cast fails, return null
    }
    
    return JsonValue();  // Default to null
}

std::string JsonConverter::getTypeName(const std::any& value) {
    if (!value.has_value()) return "null";
    
    if (value.type() == typeid(bool)) return "bool";
    if (value.type() == typeid(uint8_t)) return "uint8_t";
    if (value.type() == typeid(int8_t)) return "int8_t";
    if (value.type() == typeid(uint16_t)) return "uint16_t";
    if (value.type() == typeid(int16_t)) return "int16_t";
    if (value.type() == typeid(uint32_t)) return "uint32_t";
    if (value.type() == typeid(int32_t)) return "int32_t";
    if (value.type() == typeid(uint64_t)) return "uint64_t";
    if (value.type() == typeid(int64_t)) return "int64_t";
    if (value.type() == typeid(float)) return "float";
    if (value.type() == typeid(double)) return "double";
    
    if (value.type() == typeid(std::vector<uint8_t>)) {
        auto vec = std::any_cast<std::vector<uint8_t>>(value);
        return isCharArray(vec) ? "char[]" : "uint8_t[]";
    }
    if (value.type() == typeid(std::vector<uint16_t>)) return "uint16_t[]";
    if (value.type() == typeid(std::vector<uint32_t>)) return "uint32_t[]";
    if (value.type() == typeid(std::vector<uint64_t>)) return "uint64_t[]";
    if (value.type() == typeid(std::vector<float>)) return "float[]";
    if (value.type() == typeid(std::vector<double>)) return "double[]";
    
    return "unknown";
}

bool JsonConverter::isCharArray(const std::vector<uint8_t>& vec) {
    if (vec.empty()) return false;
    
    // Check if it contains a null terminator
    bool has_null = false;
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] == 0) {
            has_null = true;
            // Check if null terminator is at the end or followed only by zeros
            for (size_t j = i + 1; j < vec.size(); j++) {
                if (vec[j] != 0) return false;
            }
            break;
        }
        // Check if it's a printable character or common control character
        if (vec[i] < 32 || vec[i] > 126) {
            // Allow common control characters
            if (vec[i] != '\t' && vec[i] != '\n' && vec[i] != '\r' && 
                vec[i] != '\b' && vec[i] != '\f') {
                return false;
            }
        }
    }
    
    return has_null || vec.size() <= 256;  // Assume small arrays might be strings
}

std::string JsonConverter::charArrayToString(const std::vector<uint8_t>& vec) {
    std::string result;
    for (uint8_t ch : vec) {
        if (ch == 0) break;  // Stop at null terminator
        result += static_cast<char>(ch);
    }
    return result;
}

} // namespace binary_parser