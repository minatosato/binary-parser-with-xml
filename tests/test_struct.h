#include <stdint.h>

struct TestStruct {
    uint32_t header;
    uint16_t version;
    union {
        uint32_t int_data;
        float float_data;
    } value;
    uint8_t flags;
};