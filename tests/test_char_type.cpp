#include <iostream>
#include <cassert>
#include <cstring>
#include "binary_parser.h"
#include "xml_struct_parser.h"

using namespace binary_parser;

// Test that char type is correctly parsed
void test_parse_char_type() {
    std::cout << "TEST: ParseCharType ... ";
    
    // Prepare test data
    uint8_t data[] = {
        'H', 'e', 'l', 'l', 'o'  // "Hello"
    };
    
    // Create struct definition
    auto struct_info = std::make_unique<StructInfo>();
    struct_info->name = "TestStruct";
    struct_info->size = 5;
    
    auto field = std::make_unique<FieldInfo>();
    field->name = "text";
    field->type = FieldType::CHAR;
    field->offset = 0;
    field->size = 5;
    field->array_size = 5;
    struct_info->fields.push_back(std::move(field));
    
    // Parse
    BinaryParser parser;
    auto result = parser.parse(data, sizeof(data), *struct_info);
    
    assert(result != nullptr);
    assert(result->fields.count("text") == 1);
    
    // Should get vector of chars
    auto chars = std::any_cast<std::vector<uint8_t>>(result->fields["text"].value);
    assert(chars.size() == 5);
    assert(chars[0] == 'H');
    assert(chars[1] == 'e');
    assert(chars[2] == 'l');
    assert(chars[3] == 'l');
    assert(chars[4] == 'o');
    
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "Running char type tests..." << std::endl;
    
    try {
        test_parse_char_type();
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}