#include <iostream>
#include <fstream>
#include <cstring>

int main() {
    // Create test binary data with bitfields
    // Layout: flag1(1) | flag2(1) | value(14) | reserved(16) | next_byte
    // Binary: 1 | 1 | 00000000101010 | 0000000011111111 | 0xAB
    //       = 11 00000000101010 0000000011111111
    //       = 0xC02A00FF
    uint32_t bitfield_data = 0xC02A00FF;
    uint8_t next_byte = 0xAB;
    
    std::ofstream file("bitfield_test.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(&bitfield_data), sizeof(bitfield_data));
    file.write(reinterpret_cast<const char*>(&next_byte), sizeof(next_byte));
    file.close();
    
    std::cout << "Created bitfield_test.bin with:" << std::endl;
    std::cout << "  flag1 = 1" << std::endl;
    std::cout << "  flag2 = 1" << std::endl;
    std::cout << "  value = 42 (0x2A)" << std::endl;
    std::cout << "  reserved = 255 (0xFF)" << std::endl;
    std::cout << "  next_byte = 0xAB" << std::endl;
    
    return 0;
}