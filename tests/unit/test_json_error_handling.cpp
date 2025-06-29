#include <gtest/gtest.h>
#include "json/json_value.h"
#include "binary_parser/json_converter.h"
#include "binary_parser/binary_parser.h"
#include "binary_parser/xml_struct_parser.h"
#include <stdexcept>

class JsonErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = std::make_unique<binary_parser::JsonConverter>();
    }
    
    std::unique_ptr<binary_parser::JsonConverter> converter;
};

// Test error handling for invalid array access
TEST_F(JsonErrorHandlingTest, InvalidArrayAccess) {
    JsonValue arr = JsonValue::createArray();
    arr.pushBack(1);
    arr.pushBack(2);
    
    // Valid access
    EXPECT_NO_THROW(arr[0]);
    EXPECT_NO_THROW(arr[1]);
    
    // Invalid access
    EXPECT_THROW(arr[2], std::out_of_range);
    EXPECT_THROW(arr[100], std::out_of_range);
}

// Test error handling for invalid object access
TEST_F(JsonErrorHandlingTest, InvalidObjectAccess) {
    JsonValue obj = JsonValue::createObject();
    obj.set("key1", "value1");
    
    // Valid access
    EXPECT_NO_THROW(obj["key1"]);
    
    // Invalid access on const object
    const JsonValue& constObj = obj;
    EXPECT_THROW(constObj["nonexistent"], std::out_of_range);
}

// Test type mismatch errors
TEST_F(JsonErrorHandlingTest, TypeMismatchErrors) {
    JsonValue num(42);
    JsonValue str("hello");
    JsonValue arr = JsonValue::createArray();
    JsonValue obj = JsonValue::createObject();
    
    // Wrong type access
    EXPECT_THROW(num.getString(), std::runtime_error);
    EXPECT_THROW(str.getNumber(), std::runtime_error);
    EXPECT_THROW(arr.getBool(), std::runtime_error);
    EXPECT_THROW(obj.getArray(), std::runtime_error);
}

// Test file I/O errors
TEST_F(JsonErrorHandlingTest, FileIOErrors) {
    JsonValue obj = JsonValue::createObject();
    obj.set("test", "value");
    
    // Try to write to invalid path
    EXPECT_THROW(obj.writeToFile("/invalid/path/that/does/not/exist/test.json"), std::runtime_error);
    
    // Try to read non-existent file
    EXPECT_THROW(JsonValue::parseFile("/non/existent/file.json"), std::runtime_error);
}

// Test edge cases for number conversion
TEST_F(JsonErrorHandlingTest, NumberEdgeCases) {
    // Test very large numbers
    JsonValue largeNum(std::numeric_limits<double>::max());
    EXPECT_EQ(largeNum.getNumber(), std::numeric_limits<double>::max());
    
    // Test very small numbers
    JsonValue smallNum(std::numeric_limits<double>::min());
    EXPECT_EQ(smallNum.getNumber(), std::numeric_limits<double>::min());
    
    // Test infinity
    JsonValue infNum(std::numeric_limits<double>::infinity());
    EXPECT_EQ(infNum.getNumber(), std::numeric_limits<double>::infinity());
    
    // Test NaN
    JsonValue nanNum(std::numeric_limits<double>::quiet_NaN());
    EXPECT_TRUE(std::isnan(nanNum.getNumber()));
}

// Test string escaping edge cases
TEST_F(JsonErrorHandlingTest, StringEscapingEdgeCases) {
    // Test all escape sequences
    JsonValue str1("Hello\tWorld\n");
    EXPECT_TRUE(str1.toString().find("\\t") != std::string::npos);
    EXPECT_TRUE(str1.toString().find("\\n") != std::string::npos);
    
    // Test control characters
    JsonValue str2(std::string("\x01\x02\x03"));
    std::string json = str2.toString();
    EXPECT_TRUE(json.find("\\u0001") != std::string::npos);
    EXPECT_TRUE(json.find("\\u0002") != std::string::npos);
    EXPECT_TRUE(json.find("\\u0003") != std::string::npos);
}

// Test empty structures
TEST_F(JsonErrorHandlingTest, EmptyStructures) {
    // Empty array
    JsonValue emptyArr = JsonValue::createArray();
    EXPECT_EQ(emptyArr.toString(), "[]");
    EXPECT_EQ(emptyArr.size(), 0);
    
    // Empty object
    JsonValue emptyObj = JsonValue::createObject();
    EXPECT_EQ(emptyObj.toString(), "{}");
}

// Test null values
TEST_F(JsonErrorHandlingTest, NullValues) {
    JsonValue nullVal;
    EXPECT_EQ(nullVal.toString(), "null");
    EXPECT_EQ(nullVal.getType(), JsonValue::Type::NULL_TYPE);
    
    // Null in array
    JsonValue arr = JsonValue::createArray();
    arr.pushBack(JsonValue());
    EXPECT_EQ(arr.toString(), "[null]");
    
    // Null in object
    JsonValue obj = JsonValue::createObject();
    obj.set("nullKey", JsonValue());
    EXPECT_EQ(obj.toString(), "{\"nullKey\":null}");
}