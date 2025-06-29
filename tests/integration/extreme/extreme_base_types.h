#ifndef EXTREME_BASE_TYPES_H
#define EXTREME_BASE_TYPES_H

#include <stdint.h>

// Base type with anonymous union
typedef struct {
    uint16_t id;
    union {
        uint32_t raw;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };
} ColorData;

// Bitfield heavy structure
typedef struct {
    uint32_t enable : 1;
    uint32_t mode : 3;
    uint32_t level : 4;
    uint32_t reserved1 : 8;
    uint32_t flags : 16;
} ControlBits;

// Mixed anonymous structures
typedef struct {
    uint8_t type;
    union {
        struct {
            uint16_t x;
            uint16_t y;
        };
        uint32_t combined;
        uint8_t bytes[4];
    };
    uint8_t padding[3];
} MixedData;

#endif // EXTREME_BASE_TYPES_H