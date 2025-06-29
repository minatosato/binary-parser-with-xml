#include <iostream>
#include <cassert>
#include <cstring>
#include "binary_parser.h"
#include "xml_struct_parser.h"

using namespace binary_parser;

// Test for signed integer types
void test_parse_signed_int8() {
    std::cout << "TEST: ParseSignedInt8 ... ";
    
    // Prepare test data with negative value
    uint8_t data[] = {
        0xFF  // -1 in signed int8
    };
    
    // Create struct definition
    auto struct_info = std::make_unique<StructInfo>();
    struct_info->name = "TestStruct";
    struct_info->size = 1;
    
    auto field = std::make_unique<FieldInfo>();
    field->name = "value";
    field->type = FieldType::INT8;
    field->offset = 0;
    field->size = 1;
    struct_info->fields.push_back(std::move(field));
    
    // Parse
    BinaryParser parser;
    auto result = parser.parse(data, sizeof(data), *struct_info);
    
    assert(result != nullptr);
    assert(result->fields.count("value") == 1);
    
    auto value = std::any_cast<int8_t>(result->fields["value"].value);
    assert(value == -1);
    
    std::cout << "PASSED" << std::endl;
}

void test_parse_signed_int32() {
    std::cout << "TEST: ParseSignedInt32 ... ";
    
    // Prepare test data with negative value (-123456)
    uint8_t data[] = {
        0xC0, 0x1D, 0xFE, 0xFF  // -123456 in little endian
    };
    
    // Create struct definition
    auto struct_info = std::make_unique<StructInfo>();
    struct_info->name = "TestStruct";
    struct_info->size = 4;
    
    auto field = std::make_unique<FieldInfo>();
    field->name = "value";
    field->type = FieldType::INT32;
    field->offset = 0;
    field->size = 4;
    struct_info->fields.push_back(std::move(field));
    
    // Parse
    BinaryParser parser;
    auto result = parser.parse(data, sizeof(data), *struct_info);
    
    assert(result != nullptr);
    assert(result->fields.count("value") == 1);
    
    auto value = std::any_cast<int32_t>(result->fields["value"].value);
    assert(value == -123456);
    
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "Running signed types tests..." << std::endl;
    
    try {
        test_parse_signed_int8();
        test_parse_signed_int32();
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}