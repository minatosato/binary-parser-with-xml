#include <gtest/gtest.h>
#include "binary_parser/binary_parser.h"
#include "binary_parser/xml_struct_parser.h"

using namespace binary_parser;

TEST(EndiannessTest, ParseBigEndianUint32) {
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
    
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->fields.count("value"), 1);
    
    auto value = std::any_cast<uint32_t>(result->fields["value"].value);
    EXPECT_EQ(value, 0xDEADBEEF);
}
