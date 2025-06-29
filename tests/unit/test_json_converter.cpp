#include <gtest/gtest.h>
#include "binary_parser/binary_parser.h"
#include "binary_parser/xml_struct_parser.h"
#include "binary_parser/json_converter.h"
#include "json/json_value.h"

using namespace binary_parser;

// Test helper to create a simple ParsedStruct
std::unique_ptr<ParsedStruct> createSimpleParsedStruct() {
    auto parsed = std::make_unique<ParsedStruct>();
    parsed->struct_name = "TestStruct";
    
    // Add uint32_t field
    ParsedField id_field;
    id_field.name = "id";
    id_field.value = uint32_t(12345);
    parsed->fields["id"] = id_field;
    
    // Add string field
    ParsedField name_field;
    name_field.name = "name";
    std::vector<uint8_t> name_data = {'J', 'o', 'h', 'n', '\0'};
    name_field.value = name_data;
    parsed->fields["name"] = name_field;
    
    // Add bool field (as uint8_t)
    ParsedField active_field;
    active_field.name = "active";
    active_field.value = uint8_t(1);
    parsed->fields["active"] = active_field;
    
    return parsed;
}

// Test helper to create a ParsedStruct with nested structures
std::unique_ptr<ParsedStruct> createNestedParsedStruct() {
    auto parsed = std::make_unique<ParsedStruct>();
    parsed->struct_name = "PersonStruct";
    
    // Add simple fields
    ParsedField id_field;
    id_field.name = "id";
    id_field.value = uint32_t(1001);
    parsed->fields["id"] = id_field;
    
    // Add nested struct (position)
    ParsedField position_field;
    position_field.name = "position";
    
    ParsedField x_field;
    x_field.name = "x";
    x_field.value = uint16_t(100);
    position_field.sub_fields["x"] = x_field;
    
    ParsedField y_field;
    y_field.name = "y";
    y_field.value = uint16_t(200);
    position_field.sub_fields["y"] = y_field;
    
    parsed->fields["position"] = position_field;
    
    // Add array field
    ParsedField data_field;
    data_field.name = "data";
    std::vector<uint32_t> data_values = {10, 20, 30, 40};
    data_field.value = data_values;
    parsed->fields["data"] = data_field;
    
    return parsed;
}

TEST(JsonConverterTest, ConvertSimpleStruct) {
    auto parsed = createSimpleParsedStruct();
    
    JsonConverter converter;
    JsonValue json = converter.convert(*parsed);
    
    EXPECT_EQ(json.getType(), JsonValue::Type::OBJECT);
    
    // Direct field access - simple JSON format
    EXPECT_EQ(json["id"].getNumber(), 12345);
    EXPECT_EQ(json["name"].getString(), "John");
    EXPECT_EQ(json["active"].getNumber(), 1);
}

TEST(JsonConverterTest, ConvertNestedStruct) {
    auto parsed = createNestedParsedStruct();
    
    JsonConverter converter;
    JsonValue json = converter.convert(*parsed);
    
    EXPECT_EQ(json.getType(), JsonValue::Type::OBJECT);
    
    // Check simple field
    EXPECT_EQ(json["id"].getNumber(), 1001);
    
    // Check nested struct - direct access
    EXPECT_EQ(json["position"]["x"].getNumber(), 100);
    EXPECT_EQ(json["position"]["y"].getNumber(), 200);
    
    // Check array field
    auto& data_array = json["data"];
    EXPECT_EQ(data_array.getType(), JsonValue::Type::ARRAY);
    EXPECT_EQ(data_array.size(), 4);
    EXPECT_EQ(data_array[0].getNumber(), 10);
    EXPECT_EQ(data_array[1].getNumber(), 20);
    EXPECT_EQ(data_array[2].getNumber(), 30);
    EXPECT_EQ(data_array[3].getNumber(), 40);
}

TEST(JsonConverterTest, ConvertWithOptions) {
    auto parsed = createSimpleParsedStruct();
    
    JsonConverter converter;
    JsonConvertOptions options;
    options.compact = false;  // For future pretty print support
    options.include_type_info = true;
    
    JsonValue json = converter.convert(*parsed, options);
    
    // Simple format doesn't use options currently
    // Just verify basic conversion still works
    EXPECT_EQ(json["id"].getNumber(), 12345);
}

TEST(JsonConverterTest, ConvertEmptyStruct) {
    auto parsed = std::make_unique<ParsedStruct>();
    parsed->struct_name = "EmptyStruct";
    
    JsonConverter converter;
    JsonValue json = converter.convert(*parsed);
    
    // Empty struct should produce empty object
    EXPECT_EQ(json.getType(), JsonValue::Type::OBJECT);
    auto obj = json.getObject();
    EXPECT_EQ(obj.size(), 0);
}