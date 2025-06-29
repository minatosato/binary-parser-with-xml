#include <gtest/gtest.h>
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
    
    JsonValue false_value(false);
    EXPECT_EQ(false_value.getType(), JsonValue::Type::BOOL);
    EXPECT_EQ(false_value.toString(), "false");
}

TEST(JsonValueTest, NumberValue) {
    JsonValue int_value(42);
    EXPECT_EQ(int_value.getType(), JsonValue::Type::NUMBER);
    EXPECT_EQ(int_value.toString(), "42");
    
    JsonValue float_value(3.14);
    EXPECT_EQ(float_value.getType(), JsonValue::Type::NUMBER);
    EXPECT_EQ(float_value.toString(), "3.14");
}

TEST(JsonValueTest, StringValue) {
    JsonValue str_value("hello");
    EXPECT_EQ(str_value.getType(), JsonValue::Type::STRING);
    EXPECT_EQ(str_value.toString(), "\"hello\"");
    
    // エスケープが必要な文字列のテスト
    JsonValue escaped_str("hello\nworld");
    EXPECT_EQ(escaped_str.toString(), "\"hello\\nworld\"");
}
