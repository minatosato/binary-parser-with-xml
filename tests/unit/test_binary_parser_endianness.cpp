#include <iostream>
#include <cassert>
#include <cstring>
#include "binary_parser.h"
#include "xml_struct_parser.h"

using namespace binary_parser;

// Simple test framework
void test_parse_big_endian_uint32() {
    std::cout << "TEST: ParseBigEndianUint32 ... ";
    
    // Prepare test data in big endian format
    uint8_t big_endian_data[] = {
        0xDE, 0xAD, 0xBE, 0xEF  // 0xDEADBEEF in big endian
    };
    
    // Create a simple struct definition
    auto struct_info = std::make_unique<StructInfo>();
    struct_info->name = "TestStruct";
    struct_info->size = 4;
    
    auto field = std::make_unique<FieldInfo>();
    field->name = "value";
    field->type = FieldType::UINT32;
    field->offset = 0;
    field->size = 4;
    struct_info->fields.push_back(std::move(field));
    
    // Parse with big endian parser
    BinaryParser parser(Endianness::BIG);
    auto result = parser.parse(big_endian_data, sizeof(big_endian_data), *struct_info);
    
    assert(result != nullptr);
    assert(result->fields.count("value") == 1);
    
    auto value = std::any_cast<uint32_t>(result->fields["value"].value);
    assert(value == 0xDEADBEEF);
    
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "Running endianness tests..." << std::endl;
    
    try {
        test_parse_big_endian_uint32();
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}