#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <vector>
#include <map>
#include <any>
#include <cstddef>  // for offsetof
#include "extreme_test.h"
#include "../../../src/binary_parser/binary_parser.h"
#include "../../../src/binary_parser/xml_struct_parser.h"

void printIndent(int level) {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
}

void printFieldValue(const std::string& name, const std::any& value, int indent = 0) {
    printIndent(indent);
    std::cout << name << ": ";
    
    if (value.type() == typeid(uint8_t)) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(std::any_cast<uint8_t>(value)) << std::dec;
    } else if (value.type() == typeid(uint16_t)) {
        std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') 
                  << std::any_cast<uint16_t>(value) << std::dec;
    } else if (value.type() == typeid(uint32_t)) {
        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') 
                  << std::any_cast<uint32_t>(value) << std::dec;
    } else if (value.type() == typeid(uint64_t)) {
        std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') 
                  << std::any_cast<uint64_t>(value) << std::dec;
    } else if (value.type() == typeid(std::vector<std::any>)) {
        auto vec = std::any_cast<std::vector<std::any>>(value);
        std::cout << "[" << vec.size() << " elements]";
    } else if (value.type() == typeid(std::map<std::string, std::any>)) {
        std::cout << "{ nested structure }";
    } else {
        std::cout << "<unknown type>";
    }
    std::cout << std::endl;
}

void printParsedField(const binary_parser::ParsedField& field, int indent = 0) {
    printIndent(indent);
    std::cout << field.name << ": ";
    
    if (!field.sub_fields.empty()) {
        std::cout << std::endl;
        for (const auto& [name, sub_field] : field.sub_fields) {
            printParsedField(sub_field, indent + 1);
        }
    } else if (field.value.has_value()) {
        printFieldValue(field.name, field.value, 0);
    } else {
        std::cout << "<no value>" << std::endl;
    }
}

void printParsedStruct(const binary_parser::ParsedStruct& parsed) {
    std::cout << "Struct: " << parsed.struct_name << std::endl;
    for (const auto& [name, field] : parsed.fields) {
        printParsedField(field, 1);
    }
}

int main() {
    try {
        // Create extreme test data
        ExtremeTest test_data = {};
        
        // Set signature
        test_data.signature = 0xDEADBEEF;
        
        // Set timestamp in the first anonymous union
        test_data.timestamp = 0x123456789ABCDEF0;
        
        // Set some nested data
        test_data.complex_data.header = 0xCAFEBABE;
        test_data.complex_data.version = 0x3;
        test_data.complex_data.type = 0x7;
        test_data.complex_data.flags = 0xFF;
        test_data.complex_data.priority = 0x42;
        
        // Set color data in layers
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 4; ++j) {
                test_data.complex_data.layers[i].colors[j].id = (i * 4 + j) | 0x8000;
                test_data.complex_data.layers[i].colors[j].r = 0x10 + i * 4 + j;
                test_data.complex_data.layers[i].colors[j].g = 0x20 + i * 4 + j;
                test_data.complex_data.layers[i].colors[j].b = 0x30 + i * 4 + j;
                test_data.complex_data.layers[i].colors[j].a = 0xFF;
            }
            test_data.complex_data.layers[i].config.control.enable = 1;
            test_data.complex_data.layers[i].config.control.mode = i + 1;
            test_data.complex_data.layers[i].config.control.level = 0xF - i;
        }
        
        // Set the final footer
        test_data.footer.crc32 = 0x12345678;
        test_data.footer.length = 0xABCD;
        test_data.footer.type = 0x5;
        test_data.footer.priority = 0x3;
        test_data.footer.encrypted = 1;
        test_data.footer.valid = 1;
        
        // Write binary data
        std::ofstream outfile("extreme_test.bin", std::ios::binary);
        outfile.write(reinterpret_cast<const char*>(&test_data), sizeof(test_data));
        outfile.close();
        
        std::cout << "Created extreme test binary file (size: " << sizeof(test_data) << " bytes)" << std::endl;
        std::cout << "Structure offsets:" << std::endl;
        std::cout << "  signature: " << offsetof(ExtremeTest, signature) << std::endl;
        std::cout << "  anonymous union: " << offsetof(ExtremeTest, timestamp) << std::endl;
        std::cout << "  Size of ExtremeTest: " << sizeof(ExtremeTest) << std::endl;
        
        // Now parse it
        std::cout << "\nParsing binary data with XML definition..." << std::endl;
        
        binary_parser::XmlStructParser xml_parser;
        auto struct_info = xml_parser.parse("simple_extreme_test.xml");
        if (!struct_info) {
            std::cerr << "Failed to load XML definition" << std::endl;
            return 1;
        }
        
        std::cout << "Loaded XML struct size: " << struct_info->size << " bytes" << std::endl;
        std::cout << "Actual struct size: " << sizeof(ExtremeTest) << " bytes" << std::endl;
        
        // Update struct size to match actual
        struct_info->size = sizeof(ExtremeTest);
        
        // Read the binary data back
        std::ifstream infile("extreme_test.bin", std::ios::binary);
        std::vector<uint8_t> binary_data((std::istreambuf_iterator<char>(infile)),
                                        std::istreambuf_iterator<char>());
        infile.close();
        
        // Parse the data
        binary_parser::BinaryParser parser;
        auto result = parser.parse(binary_data.data(), binary_data.size(), *struct_info);
        
        std::cout << "\nParsed data:" << std::endl;
        if (result) {
            printParsedStruct(*result);
        } else {
            std::cerr << "Failed to parse binary data" << std::endl;
            return 1;
        }
        
        // Verify some key values
        std::cout << "\nVerifying key values..." << std::endl;
        
        if (!result) {
            std::cerr << "Result is null!" << std::endl;
            return 1;
        }
        
        // Check signature
        if (result->fields.count("signature")) {
            const auto& sig_field = result->fields.at("signature");
            if (sig_field.value.has_value() && sig_field.value.type() == typeid(uint32_t)) {
                uint32_t sig = std::any_cast<uint32_t>(sig_field.value);
                std::cout << "✓ Signature: 0x" << std::hex << sig << std::dec 
                          << (sig == 0xDEADBEEF ? " (correct)" : " (INCORRECT!)") << std::endl;
            } else {
                std::cout << "✗ Signature not found or wrong type!" << std::endl;
            }
        } else {
            std::cout << "✗ Signature field not found!" << std::endl;
        }
        
        // Check nested anonymous union
        if (result->fields.count("unnamed")) {
            const auto& unnamed_field = result->fields.at("unnamed");
            
            // In a union, we should see multiple fields at the same offset
            if (unnamed_field.sub_fields.count("timestamp")) {
                const auto& ts_field = unnamed_field.sub_fields.at("timestamp");
                if (ts_field.value.has_value() && ts_field.value.type() == typeid(uint64_t)) {
                    uint64_t ts = std::any_cast<uint64_t>(ts_field.value);
                    std::cout << "✓ Timestamp: 0x" << std::hex << ts << std::dec 
                              << (ts == 0x123456789ABCDEF0 ? " (correct)" : " (INCORRECT!)") << std::endl;
                }
            }
            
            // Check complex_data (which overlaps with timestamp in the union)
            if (unnamed_field.sub_fields.count("complex_data")) {
                std::cout << "✓ Found complex_data in union" << std::endl;
            }
        }
        
        std::cout << "\nExtreme test completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}