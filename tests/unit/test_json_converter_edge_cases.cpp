#include <gtest/gtest.h>
#include "binary_parser/json_converter.h"
#include "binary_parser/binary_parser.h"
#include <limits>
#include <cmath>

class JsonConverterEdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = std::make_unique<binary_parser::JsonConverter>();
    }
    
    std::unique_ptr<binary_parser::JsonConverter> converter;
};

// Test conversion of extreme numeric values
TEST_F(JsonConverterEdgeCasesTest, ExtremeNumericValues) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "ExtremeValues";
    
    // Maximum values
    parsed.fields["max_uint8"].value = std::numeric_limits<uint8_t>::max();
    parsed.fields["max_uint16"].value = std::numeric_limits<uint16_t>::max();
    parsed.fields["max_uint32"].value = std::numeric_limits<uint32_t>::max();
    parsed.fields["max_uint64"].value = std::numeric_limits<uint64_t>::max();
    
    // Minimum values
    parsed.fields["min_int8"].value = std::numeric_limits<int8_t>::min();
    parsed.fields["min_int16"].value = std::numeric_limits<int16_t>::min();
    parsed.fields["min_int32"].value = std::numeric_limits<int32_t>::min();
    parsed.fields["min_int64"].value = std::numeric_limits<int64_t>::min();
    
    // Float extremes
    parsed.fields["max_float"].value = std::numeric_limits<float>::max();
    parsed.fields["min_float"].value = std::numeric_limits<float>::min();
    parsed.fields["inf_float"].value = std::numeric_limits<float>::infinity();
    parsed.fields["nan_float"].value = std::numeric_limits<float>::quiet_NaN();
    
    JsonValue json = converter->convert(parsed);
    
    // Verify conversions
    EXPECT_EQ(json["max_uint8"].getNumber(), 255);
    EXPECT_EQ(json["max_uint16"].getNumber(), 65535);
    EXPECT_EQ(json["max_uint32"].getNumber(), 4294967295.0);
    EXPECT_GT(json["max_uint64"].getNumber(), 1e18); // Very large number
    
    EXPECT_EQ(json["min_int8"].getNumber(), -128);
    EXPECT_EQ(json["min_int16"].getNumber(), -32768);
    EXPECT_EQ(json["min_int32"].getNumber(), -2147483648.0);
    
    // Check special float values
    EXPECT_GT(json["max_float"].getNumber(), 1e38);
    EXPECT_GT(json["min_float"].getNumber(), 0);
    EXPECT_TRUE(std::isinf(json["inf_float"].getNumber()));
    EXPECT_TRUE(std::isnan(json["nan_float"].getNumber()));
}

// Test empty arrays
TEST_F(JsonConverterEdgeCasesTest, EmptyArrays) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "EmptyArrays";
    
    // Empty uint8 array
    parsed.fields["empty_bytes"].value = std::vector<uint8_t>{};
    
    // Empty uint32 array
    parsed.fields["empty_ints"].value = std::vector<uint32_t>{};
    
    JsonValue json = converter->convert(parsed);
    
    EXPECT_EQ(json["empty_bytes"].toString(), "[]");
    EXPECT_EQ(json["empty_ints"].toString(), "[]");
}

// Test very long strings
TEST_F(JsonConverterEdgeCasesTest, VeryLongStrings) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "LongStrings";
    
    // Create a very long string
    std::vector<uint8_t> longString(1000);
    std::fill(longString.begin(), longString.end() - 1, 'A');
    longString[999] = '\0';
    parsed.fields["long_string"].value = longString;
    
    JsonValue json = converter->convert(parsed);
    
    std::string result = json["long_string"].getString();
    EXPECT_EQ(result.length(), 999);
    EXPECT_EQ(result[0], 'A');
    EXPECT_EQ(result[998], 'A');
}

// Test strings with special characters
TEST_F(JsonConverterEdgeCasesTest, SpecialCharacterStrings) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "SpecialChars";
    
    // String with various special characters
    std::vector<uint8_t> specialStr = {
        'H', 'e', 'l', 'l', 'o', '\t', 'W', 'o', 'r', 'l', 'd', '\n',
        '"', '\\', '/', '\r', '\b', '\f', '\0'
    };
    parsed.fields["special"].value = specialStr;
    
    JsonValue json = converter->convert(parsed);
    
    // Get the actual string value and check it contains the expected content
    std::string strValue = json["special"].getString();
    EXPECT_EQ(strValue, "Hello\tWorld\n\"\\/\r\b\f");
    
    // Check that the JSON representation has proper escaping
    std::string jsonStr = json["special"].toString();
    EXPECT_TRUE(jsonStr.find("\\t") != std::string::npos);
    EXPECT_TRUE(jsonStr.find("\\n") != std::string::npos);
    EXPECT_TRUE(jsonStr.find("\\\"") != std::string::npos);
    EXPECT_TRUE(jsonStr.find("\\\\") != std::string::npos);
}

// Test deeply nested structures
TEST_F(JsonConverterEdgeCasesTest, DeeplyNestedStructures) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "NestedStruct";
    
    // Create nested structure
    binary_parser::ParsedField level1;
    level1.name = "level1";
    
    binary_parser::ParsedField level2;
    level2.name = "level2";
    level2.value = uint32_t(42);
    
    level1.sub_fields["level2"] = level2;
    parsed.fields["nested"] = level1;
    
    JsonValue json = converter->convert(parsed);
    
    EXPECT_EQ(json["nested"]["level2"].getNumber(), 42);
}

// Test mixed array types (struct arrays)
TEST_F(JsonConverterEdgeCasesTest, StructArrays) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "StructWithArrays";
    
    // Create an array of ParsedField (simulating struct array)
    std::vector<std::any> structArray;
    
    for (int i = 0; i < 3; ++i) {
        binary_parser::ParsedField element;
        element.name = std::to_string(i);
        element.sub_fields["x"].value = int32_t(i * 10);
        element.sub_fields["y"].value = int32_t(i * 20);
        structArray.push_back(element);
    }
    
    parsed.fields["points"].value = structArray;
    
    JsonValue json = converter->convert(parsed);
    
    // Verify struct array conversion
    EXPECT_EQ(json["points"][0]["x"].getNumber(), 0);
    EXPECT_EQ(json["points"][1]["x"].getNumber(), 10);
    EXPECT_EQ(json["points"][2]["x"].getNumber(), 20);
}

// Test char arrays with non-printable characters
TEST_F(JsonConverterEdgeCasesTest, NonPrintableCharArrays) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "NonPrintable";
    
    // Array with non-printable characters (should not be converted to string)
    std::vector<uint8_t> nonPrintable = {0x01, 0x02, 0x03, 0x04, 0x05};
    parsed.fields["binary_data"].value = nonPrintable;
    
    // Array with mostly printable but some non-printable (should not be converted)
    std::vector<uint8_t> mixed = {'H', 'e', 'l', 'l', 'o', 0x01, 'W', 'o', 'r', 'l', 'd', '\0'};
    parsed.fields["mixed_data"].value = mixed;
    
    JsonValue json = converter->convert(parsed);
    
    // Should be arrays, not strings
    EXPECT_EQ(json["binary_data"][0].getNumber(), 1);
    EXPECT_EQ(json["binary_data"][1].getNumber(), 2);
    EXPECT_EQ(json["mixed_data"][0].getNumber(), 72); // 'H'
    EXPECT_EQ(json["mixed_data"][5].getNumber(), 1);  // 0x01
}

// Test empty struct
TEST_F(JsonConverterEdgeCasesTest, EmptyStruct) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "EmptyStruct";
    // No fields
    
    JsonValue json = converter->convert(parsed);
    EXPECT_EQ(json.toString(), "{}");
}

// Test field with no value (empty std::any)
TEST_F(JsonConverterEdgeCasesTest, EmptyFieldValue) {
    binary_parser::ParsedStruct parsed;
    parsed.struct_name = "StructWithEmptyField";
    
    binary_parser::ParsedField emptyField;
    emptyField.name = "empty";
    // emptyField.value is not set (empty std::any)
    
    parsed.fields["empty_field"] = emptyField;
    parsed.fields["normal_field"].value = uint32_t(123);
    
    JsonValue json = converter->convert(parsed);
    
    EXPECT_EQ(json["empty_field"].getType(), JsonValue::Type::NULL_TYPE);
    EXPECT_EQ(json["normal_field"].getNumber(), 123);
}