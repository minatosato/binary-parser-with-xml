#include <iostream>
#include <fstream>
#include <cstring>
#include "complex_test.h"

int main() {
    ComplexTest test_data = {};
    
    // Set values
    test_data.header = 0x12345678;
    
    // Array values
    test_data.values[0] = 0x1111;
    test_data.values[1] = 0x2222;
    test_data.values[2] = 0x3333;
    test_data.values[3] = 0x4444;
    
    // Bitfields in anonymous struct
    test_data.flag1 = 1;
    test_data.flag2 = 0;
    test_data.count = 42;
    test_data.reserved = 0xABCDEF;
    
    // Union data
    test_data.data.low = 0x11223344;
    test_data.data.high = 0x55667788;
    
    // Nested struct
    test_data.nested.type = 0xFF;
    test_data.nested.content.coords.x = 100;
    test_data.nested.content.coords.y = 200;
    test_data.nested.content.coords.z = 300;
    test_data.nested.content.coords.w = 400;
    
    test_data.checksum = 0xDEADBEEF;
    
    // Write to file
    std::ofstream file("complex_test.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(&test_data), sizeof(test_data));
    file.close();
    
    std::cout << "Created complex_test.bin with nested structures" << std::endl;
    std::cout << "Size of ComplexTest: " << sizeof(ComplexTest) << " bytes" << std::endl;
    
    return 0;
}