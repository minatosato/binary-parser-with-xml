#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "xml_struct_parser.h"
#include "binary_parser.h"

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <xml_file> <binary_file>\n";
    std::cout << "  xml_file    : XML struct definition file\n";
    std::cout << "  binary_file : Binary data file to parse\n";
}

void printParsedField(const binary_parser::ParsedField& field, int indent = 0) {
    std::string prefix(indent * 2, ' ');
    
    if (!field.sub_fields.empty()) {
        std::cout << prefix << field.name << ":\n";
        for (const auto& [name, sub_field] : field.sub_fields) {
            printParsedField(sub_field, indent + 1);
        }
    } else {
        std::cout << prefix << field.name << " = ";
        
        // Try to print value as different types
        try {
            if (field.value.type() == typeid(uint8_t)) {
                std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(std::any_cast<uint8_t>(field.value));
            } else if (field.value.type() == typeid(uint16_t)) {
                std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') 
                          << std::any_cast<uint16_t>(field.value);
            } else if (field.value.type() == typeid(uint32_t)) {
                std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << std::any_cast<uint32_t>(field.value);
            } else if (field.value.type() == typeid(uint64_t)) {
                std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') 
                          << std::any_cast<uint64_t>(field.value);
            } else if (field.value.type() == typeid(float)) {
                std::cout << std::dec << std::any_cast<float>(field.value);
            } else if (field.value.type() == typeid(double)) {
                std::cout << std::dec << std::any_cast<double>(field.value);
            } else {
                std::cout << "<unknown type>";
            }
        } catch (const std::bad_any_cast& e) {
            std::cout << "<cast error>";
        }
        
        std::cout << std::dec << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    const char* xml_file = argv[1];
    const char* binary_file = argv[2];
    
    try {
        // Parse XML struct definition
        binary_parser::XmlStructParser xml_parser;
        auto struct_info = xml_parser.parse(xml_file);
        
        std::cout << "Loaded struct: " << struct_info->name 
                  << " (size: " << struct_info->size << " bytes)\n\n";
        
        // Read binary file
        std::ifstream bin_file(binary_file, std::ios::binary | std::ios::ate);
        if (!bin_file) {
            std::cerr << "Error: Cannot open binary file: " << binary_file << "\n";
            return 1;
        }
        
        size_t file_size = bin_file.tellg();
        bin_file.seekg(0);
        
        std::vector<uint8_t> data(file_size);
        bin_file.read(reinterpret_cast<char*>(data.data()), file_size);
        bin_file.close();
        
        // Parse binary data
        binary_parser::BinaryParser parser;
        auto parsed = parser.parse(data.data(), data.size(), *struct_info);
        
        std::cout << "Parsed data:\n";
        for (const auto& [name, field] : parsed->fields) {
            printParsedField(field);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}