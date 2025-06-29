#include <stdint.h>

struct TestData {
    uint32_t magic;
    uint16_t version;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } color;
    union {
        uint32_t int_value;
        float float_value;
    } data;
};