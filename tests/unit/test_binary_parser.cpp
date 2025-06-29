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

TEST_F(BinaryParserTest, ParseStructWithArray) {
    // Create test XML file with array
    const char* xml_content = R"(<?xml version="1.0" ?>
<struct name="ArrayStruct" size="24">
  <field name="count" type="uint32_t" offset="0" size="4"/>
  <field name="data" type="uint32_t" offset="4" size="16" array_size="4"/>
  <field name="checksum" type="uint32_t" offset="20" size="4"/>
</struct>)";
    
    std::ofstream out("test_array_struct.xml");
    out << xml_content;
    out.close();
    
    // Parse XML
    XmlStructParser xml_parser;
    auto struct_info = xml_parser.parse("test_array_struct.xml");
    ASSERT_NE(struct_info, nullptr);
    
    // Create test binary data
    uint8_t test_data[24];
    uint32_t count = 4;
    uint32_t data_array[4] = {10, 20, 30, 40};
    uint32_t checksum = 100;
    
    std::memcpy(test_data + 0, &count, 4);
    std::memcpy(test_data + 4, data_array, 16);
    std::memcpy(test_data + 20, &checksum, 4);
    
    // Parse binary data
    BinaryParser parser;
    auto parsed = parser.parse(test_data, sizeof(test_data), *struct_info);
    ASSERT_NE(parsed, nullptr);
    
    // Verify array values
    auto array_field = parsed->fields["data"];
    auto array_values = BinaryParser::getArray<uint32_t>(array_field);
    ASSERT_EQ(array_values.size(), 4);
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(array_values[i], data_array[i]);
    }
    
    std::remove("test_array_struct.xml");
}

TEST_F(BinaryParserTest, ParseStructWithBitfield) {
    // Create test XML file with bitfields
    const char* xml_content = R"(<?xml version="1.0" ?>
<struct name="BitfieldStruct" size="4">
  <field name="flag1" type="uint32_t" offset="0" size="4" bits="1" bit_offset="0"/>
  <field name="flag2" type="uint32_t" offset="0" size="4" bits="1" bit_offset="1"/>
  <field name="value" type="uint32_t" offset="0" size="4" bits="14" bit_offset="2"/>
  <field name="reserved" type="uint32_t" offset="0" size="4" bits="16" bit_offset="16"/>
</struct>)";
    
    std::ofstream out("test_bitfield_struct.xml");
    out << xml_content;
    out.close();
    
    // Parse XML
    XmlStructParser xml_parser;
    auto struct_info = xml_parser.parse("test_bitfield_struct.xml");
    ASSERT_NE(struct_info, nullptr);
    
    // Create test binary data
    // Layout: flag1(1) | flag2(1) | value(14) | reserved(16)
    // Binary: 1 | 1 | 00000000101010 | 0000000011111111
    //       = 11 00000000101010 0000000011111111
    //       = 0xC02A00FF
    uint32_t test_data = 0x00FF00AB; // Corrected value
    
    // Parse binary data
    BinaryParser parser;
    auto parsed = parser.parse(reinterpret_cast<uint8_t*>(&test_data), sizeof(test_data), *struct_info);
    ASSERT_NE(parsed, nullptr);
    
    // Verify bitfield values
    EXPECT_EQ(BinaryParser::getValue<uint32_t>(parsed->fields["flag1"]), 1);
    EXPECT_EQ(BinaryParser::getValue<uint32_t>(parsed->fields["flag2"]), 1);
    EXPECT_EQ(BinaryParser::getValue<uint32_t>(parsed->fields["value"]), 42);  // 0x2A = 42
    EXPECT_EQ(BinaryParser::getValue<uint32_t>(parsed->fields["reserved"]), 255); // 0xFF = 255
    
    std::remove("test_bitfield_struct.xml");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}