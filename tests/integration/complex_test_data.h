#include <stdint.h>

struct ComplexTestData {
    uint32_t id;
    char name[32];
    struct {
        int32_t x;
        int32_t y;
        int32_t z;
    } position;
    uint16_t flags;
    float values[5];
    struct {
        uint8_t type;
        uint16_t count;
        uint8_t padding;
    } items[3];
};