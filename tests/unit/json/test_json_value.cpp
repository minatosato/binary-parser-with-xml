#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "json/json_value.h"

TEST(JsonValueTest, NullValue) {
    JsonValue null_value;
    EXPECT_EQ(null_value.getType(), JsonValue::Type::NULL_TYPE);
    EXPECT_EQ(null_value.toString(), "null");
}

TEST(JsonValueTest, BooleanValue) {
    JsonValue true_value(true);
    EXPECT_EQ(true_value.getType(), JsonValue::Type::BOOL);
    EXPECT_EQ(true_value.toString(), "true");
    EXPECT_EQ(true_value.getBool(), true);
    
    JsonValue false_value(false);
    EXPECT_EQ(false_value.getType(), JsonValue::Type::BOOL);
    EXPECT_EQ(false_value.toString(), "false");
    EXPECT_EQ(false_value.getBool(), false);
}

TEST(JsonValueTest, NumberValue) {
    JsonValue int_value(42);
    EXPECT_EQ(int_value.getType(), JsonValue::Type::NUMBER);
    EXPECT_EQ(int_value.toString(), "42");
    EXPECT_EQ(int_value.getNumber(), 42.0);
    
    JsonValue float_value(3.14);
    EXPECT_EQ(float_value.getType(), JsonValue::Type::NUMBER);
    EXPECT_EQ(float_value.toString(), "3.14");
    EXPECT_DOUBLE_EQ(float_value.getNumber(), 3.14);
    
    // Zero
    JsonValue zero_value(0.0);
    EXPECT_EQ(zero_value.toString(), "0");
}

TEST(JsonValueTest, StringValue) {
    JsonValue str_value("hello");
    EXPECT_EQ(str_value.getType(), JsonValue::Type::STRING);
    EXPECT_EQ(str_value.toString(), "\"hello\"");
    EXPECT_EQ(str_value.getString(), "hello");
    
    // エスケープが必要な文字列のテスト
    JsonValue escaped_str("hello\nworld");
    EXPECT_EQ(escaped_str.toString(), "\"hello\\nworld\"");
    
    // More escape tests
    JsonValue quotes_str("He said \"Hello\"");
    EXPECT_EQ(quotes_str.toString(), "\"He said \\\"Hello\\\"\"");
    
    JsonValue tabs_str("A\tB\tC");
    EXPECT_EQ(tabs_str.toString(), "\"A\\tB\\tC\"");
}

TEST(JsonValueTest, ArrayValue) {
    JsonValue array;
    array.pushBack(JsonValue(1));
    array.pushBack(JsonValue("two"));
    array.pushBack(JsonValue(true));
    
    EXPECT_EQ(array.getType(), JsonValue::Type::ARRAY);
    EXPECT_EQ(array.size(), 3);
    
    // Test array access
    EXPECT_EQ(array[0].getNumber(), 1.0);
    EXPECT_EQ(array[1].getString(), "two");
    EXPECT_EQ(array[2].getBool(), true);
    
    // Test array toString()
    EXPECT_EQ(array.toString(), "[1,\"two\",true]");
}

TEST(JsonValueTest, ObjectValue) {
    JsonValue obj;
    obj.set("name", JsonValue("John"));
    obj.set("age", JsonValue(30));
    obj.set("active", JsonValue(true));
    
    EXPECT_EQ(obj.getType(), JsonValue::Type::OBJECT);
    EXPECT_TRUE(obj.contains("name"));
    EXPECT_TRUE(obj.contains("age"));
    EXPECT_TRUE(obj.contains("active"));
    EXPECT_FALSE(obj.contains("unknown"));
    
    // Test object access
    EXPECT_EQ(obj["name"].getString(), "John");
    EXPECT_EQ(obj["age"].getNumber(), 30.0);
    EXPECT_EQ(obj["active"].getBool(), true);
    
    // Test object toString()
    // Note: order may vary in std::map
    std::string json_str = obj.toString();
    EXPECT_TRUE(json_str.find("\"name\":\"John\"") != std::string::npos);
    EXPECT_TRUE(json_str.find("\"age\":30") != std::string::npos);
    EXPECT_TRUE(json_str.find("\"active\":true") != std::string::npos);
}

TEST(JsonValueTest, NestedStructures) {
    // Nested array
    JsonValue nested_array;
    JsonValue inner_array;
    inner_array.pushBack(JsonValue(1));
    inner_array.pushBack(JsonValue(2));
    nested_array.pushBack(inner_array);
    nested_array.pushBack(JsonValue("text"));
    
    EXPECT_EQ(nested_array.size(), 2);
    EXPECT_EQ(nested_array[0].getType(), JsonValue::Type::ARRAY);
    EXPECT_EQ(nested_array[0].size(), 2);
    
    // Nested object
    JsonValue person;
    JsonValue address;
    address.set("street", JsonValue("123 Main St"));
    address.set("city", JsonValue("Boston"));
    person.set("name", JsonValue("John"));
    person.set("address", address);
    
    EXPECT_EQ(person["address"]["city"].getString(), "Boston");
}

TEST(JsonValueTest, PrettyPrint) {
    // Test pretty printing
    JsonValue obj;
    obj.set("name", JsonValue("John"));
    obj.set("age", JsonValue(30));
    
    JsonValue items;
    items.pushBack(JsonValue("apple"));
    items.pushBack(JsonValue("banana"));
    obj.set("items", items);
    
    // Test compact (default)
    std::string compact = obj.toString(false);
    EXPECT_TRUE(compact.find("\n") == std::string::npos);  // No newlines
    
    // Test pretty print
    std::string pretty = obj.toString(true);
    EXPECT_TRUE(pretty.find("\n") != std::string::npos);  // Has newlines
    EXPECT_TRUE(pretty.find("  \"name\"") != std::string::npos);  // Has indentation
    
    // Test empty object pretty print
    JsonValue empty_obj = JsonValue::createObject();
    EXPECT_EQ(empty_obj.toString(true), "{}");
    
    // Test empty array pretty print
    JsonValue empty_arr = JsonValue::createArray();
    EXPECT_EQ(empty_arr.toString(true), "[]");
}

TEST(JsonValueTest, FileIO) {
    // Test writeToFile
    JsonValue obj;
    obj.set("test", JsonValue("data"));
    obj.set("number", JsonValue(42));
    
    // Write to file
    std::string test_file = "test_json_io.json";
    obj.writeToFile(test_file);
    
    // Read file back and verify it was written
    std::ifstream file(test_file);
    EXPECT_TRUE(file.is_open());
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    
    // Should contain pretty-printed JSON
    EXPECT_TRUE(content.find("\"test\": \"data\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"number\": 42") != std::string::npos);
    
    // Clean up
    std::remove(test_file.c_str());
}
