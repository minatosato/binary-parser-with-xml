#ifndef EXTREME_MIDDLE_TYPES_H
#define EXTREME_MIDDLE_TYPES_H

#include "extreme_base_types.h"

// Complex nested structure with arrays of structs
typedef struct {
    ColorData colors[4];
    struct {
        ControlBits control;
        union {
            MixedData mixed[2];
            struct {
                uint64_t timestamp;
                uint32_t sequence;
                uint32_t checksum;
            };
        };
    } config;
} LayerData;

// Deep anonymous nesting
typedef struct {
    uint32_t header;
    struct {
        uint16_t version : 4;
        uint16_t type : 4;
        uint16_t flags : 8;
        union {
            struct {
                uint8_t priority;
                uint8_t category;
                uint16_t size;
            };
            uint32_t metadata;
        };
    };
    LayerData layers[2];
} MiddleComplex;

// Union with anonymous struct arrays
typedef union {
    struct {
        uint32_t magic;
        struct {
            uint16_t cmd;
            uint16_t arg;
        } commands[8];
    };
    struct {
        uint64_t header;
        uint8_t payload[32];
    };
    uint8_t raw_bytes[40];
} PacketUnion;

#endif // EXTREME_MIDDLE_TYPES_H