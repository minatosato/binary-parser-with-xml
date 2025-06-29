#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct {
    uint32_t x;
    uint32_t y;
} Point;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

#endif