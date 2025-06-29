#include <fstream>
#include <cstring>
#include "test_data.h"

int main() {
    TestData data;
    data.magic = 0xDEADBEEF;
    data.version = 0x0102;
    data.color.r = 255;
    data.color.g = 128;
    data.color.b = 64;
    data.color.a = 255;
    data.data.float_value = 3.14159f;
    
    std::ofstream file("test_data.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));
    file.close();
    
    return 0;
}