#include <gtest/gtest.h>
#include "binary_parser/binary_parser.h"
#include "binary_parser/xml_struct_parser.h"

using namespace binary_parser;

TEST(CharTypeTest, ParseCharType) {
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
    
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->fields.count("text"), 1);
    
    // Should get vector of chars
    auto chars = std::any_cast<std::vector<uint8_t>>(result->fields["text"].value);
    ASSERT_EQ(chars.size(), 5);
    EXPECT_EQ(chars[0], 'H');
    EXPECT_EQ(chars[1], 'e');
    EXPECT_EQ(chars[2], 'l');
    EXPECT_EQ(chars[3], 'l');
    EXPECT_EQ(chars[4], 'o');
}
