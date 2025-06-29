#include <fstream>
#include <cstring>
#include "complex_test_data.h"

int main() {
    ComplexTestData data;
    
    // Initialize data
    data.id = 12345;
    std::strcpy(data.name, "Test Object");
    
    data.position.x = 100;
    data.position.y = 200;
    data.position.z = -50;
    
    data.flags = 0x1234;
    
    data.values[0] = 1.5f;
    data.values[1] = 2.5f;
    data.values[2] = 3.14159f;
    data.values[3] = -7.25f;
    data.values[4] = 0.0f;
    
    data.items[0].type = 1;
    data.items[0].count = 10;
    data.items[0].padding = 0;
    
    data.items[1].type = 2;
    data.items[1].count = 20;
    data.items[1].padding = 0;
    
    data.items[2].type = 3;
    data.items[2].count = 30;
    data.items[2].padding = 0;
    
    // Write to binary file
    std::ofstream file("complex_test_data.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(&data), sizeof(data));
    file.close();
    
    return 0;
}