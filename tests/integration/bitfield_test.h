#include <stdint.h>

struct BitfieldTest {
    uint32_t flag1 : 1;
    uint32_t flag2 : 1;
    uint32_t value : 14;
    uint32_t reserved : 16;
    uint8_t next_byte;
};