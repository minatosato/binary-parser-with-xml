#include <gtest/gtest.h>
#include "xml_struct_parser.h"
#include "binary_parser.h"
#include <fstream>
#include <cstring>

using namespace binary_parser;

class BinaryParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test XML file
        const char* xml_content = R"(<?xml version="1.0" ?>
<struct name="TestStruct" size="12">
  <field name="magic" type="uint32_t" offset="0" size="4"/>
  <field name="version" type="uint16_t" offset="4" size="2"/>
  <field name="flags" type="uint8_t" offset="6" size="1"/>
  <field name="padding" type="uint8_t" offset="7" size="1"/>
  <field name="value" type="float" offset="8" size="4"/>
</struct>)";
        
        std::ofstream out("test_struct.xml");
        out << xml_content;
        out.close();
    }
    
    void TearDown() override {
        std::remove("test_struct.xml");
    }
};

TEST_F(BinaryParserTest, ParseSimpleStruct) {
    // Parse XML
    XmlStructParser xml_parser;
    auto struct_info = xml_parser.parse("test_struct.xml");
    ASSERT_NE(struct_info, nullptr);
    EXPECT_EQ(struct_info->name, "TestStruct");
    EXPECT_EQ(struct_info->size, 12);
    
    // Create test binary data
    uint8_t test_data[12];
    uint32_t magic = 0x12345678;
    uint16_t version = 0x0102;
    uint8_t flags = 0xFF;
    uint8_t padding = 0x00;
    float value = 3.14f;
    
    std::memcpy(test_data + 0, &magic, 4);
    std::memcpy(test_data + 4, &version, 2);
    std::memcpy(test_data + 6, &flags, 1);
    std::memcpy(test_data + 7, &padding, 1);
    std::memcpy(test_data + 8, &value, 4);
    
    // Parse binary data
    BinaryParser parser;
    auto parsed = parser.parse(test_data, sizeof(test_data), *struct_info);
    ASSERT_NE(parsed, nullptr);
    
    // Verify parsed values
    EXPECT_EQ(BinaryParser::getValue<uint32_t>(parsed->fields["magic"]), magic);
    EXPECT_EQ(BinaryParser::getValue<uint16_t>(parsed->fields["version"]), version);
    EXPECT_EQ(BinaryParser::getValue<uint8_t>(parsed->fields["flags"]), flags);
    EXPECT_FLOAT_EQ(BinaryParser::getValue<float>(parsed->fields["value"]), value);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}