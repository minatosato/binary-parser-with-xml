#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "xml_struct_parser.h"
#include "binary_parser.h"
#include "json_converter.h"
#include "../json/json_value.h"

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <xml_file> <binary_file> [options]\n";
    std::cout << "  xml_file    : XML struct definition file\n";
    std::cout << "  binary_file : Binary data file to parse\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --big-endian, -b  : Parse as big-endian (default: little-endian)\n";
    std::cout << "  --json            : Output as JSON format\n";
    std::cout << "  --pretty          : Pretty print JSON output\n";
    std::cout << "  -o <file>         : Output to file instead of stdout\n";
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
            } else if (field.value.type() == typeid(std::vector<uint8_t>)) {
                auto vec = std::any_cast<std::vector<uint8_t>>(field.value);
                std::cout << "[";
                for (size_t i = 0; i < vec.size() && i < 16; ++i) {
                    if (i > 0) std::cout << " ";
                    if (vec[i] >= 32 && vec[i] < 127) {
                        std::cout << "'" << static_cast<char>(vec[i]) << "'";
                    } else {
                        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                                  << static_cast<int>(vec[i]);
                    }
                }
                if (vec.size() > 16) std::cout << " ...";
                std::cout << "]";
            } else if (field.value.type() == typeid(std::vector<uint16_t>)) {
                auto vec = std::any_cast<std::vector<uint16_t>>(field.value);
                std::cout << "[";
                for (size_t i = 0; i < vec.size() && i < 10; ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << vec[i];
                }
                if (vec.size() > 10) std::cout << ", ...";
                std::cout << "]";
            } else if (field.value.type() == typeid(std::vector<uint32_t>)) {
                auto vec = std::any_cast<std::vector<uint32_t>>(field.value);
                std::cout << "[";
                for (size_t i = 0; i < vec.size() && i < 10; ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << vec[i];
                }
                if (vec.size() > 10) std::cout << ", ...";
                std::cout << "]";
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
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    const char* xml_file = argv[1];
    const char* binary_file = argv[2];
    
    // Parse command line options
    binary_parser::Endianness endianness = binary_parser::Endianness::LITTLE;
    bool output_json = false;
    bool pretty_print = false;
    std::string output_file;
    
    for (int i = 3; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--big-endian" || arg == "-b") {
            endianness = binary_parser::Endianness::BIG;
        } else if (arg == "--json") {
            output_json = true;
        } else if (arg == "--pretty") {
            pretty_print = true;
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        }
    }
    
    try {
        // Parse XML struct definition
        binary_parser::XmlStructParser xml_parser;
        auto struct_info = xml_parser.parse(xml_file);
        
        if (!output_json) {
            std::cout << "Loaded struct: " << struct_info->name 
                      << " (size: " << struct_info->size << " bytes)\n\n";
        }
        
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
        binary_parser::BinaryParser parser(endianness);
        auto parsed = parser.parse(data.data(), data.size(), *struct_info);
        
        if (output_json) {
            // Convert to JSON
            binary_parser::JsonConverter converter;
            binary_parser::JsonConvertOptions options;
            options.include_type_info = false;  // Can be made configurable later
            
            JsonValue json = converter.convert(*parsed, options);
            std::string json_str = json.toString(pretty_print);
            
            if (!output_file.empty()) {
                // Write to file
                std::ofstream out_file(output_file);
                if (!out_file) {
                    std::cerr << "Error: Cannot create output file: " << output_file << "\n";
                    return 1;
                }
                out_file << json_str << "\n";
                out_file.close();
            } else {
                // Write to stdout
                std::cout << json_str << "\n";
            }
        } else {
            // Traditional output
            if (endianness == binary_parser::Endianness::BIG) {
                std::cout << "Parsing as big-endian\n";
            } else {
                std::cout << "Parsing as little-endian (default)\n";
            }
            std::cout << "\n";
            
            std::cout << "Parsed data:\n";
            for (const auto& [name, field] : parsed->fields) {
                printParsedField(field);
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}