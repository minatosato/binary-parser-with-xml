#include <stdint.h>

// Complex nested structure with arrays, bitfields, unions, and anonymous structs
struct ComplexTest {
    uint32_t header;
    
    // Array of simple values
    uint16_t values[4];
    
    // Anonymous struct with bitfields
    struct {
        uint32_t flag1 : 1;
        uint32_t flag2 : 1;
        uint32_t count : 6;
        uint32_t reserved : 24;
    };
    
    // Named union with anonymous struct inside
    union DataUnion {
        uint64_t raw_data;
        struct {
            uint32_t low;
            uint32_t high;
        };
        float float_values[2];
    } data;
    
    // Nested struct with array
    struct {
        uint8_t type;
        uint8_t padding[3];
        union {
            uint32_t integer_data[2];
            struct {
                uint16_t x;
                uint16_t y;
                uint16_t z;
                uint16_t w;
            } coords;
        } content;
    } nested;
    
    uint32_t checksum;
};