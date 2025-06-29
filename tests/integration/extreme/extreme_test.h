#ifndef EXTREME_TEST_H
#define EXTREME_TEST_H

#include "extreme_middle_types.h"

// The ultimate nightmare structure
struct ExtremeTest {
    // Start with simple field
    uint32_t signature;
    
    // Anonymous union containing anonymous structs
    union {
        struct {
            uint64_t timestamp;
            union {
                MiddleComplex complex_data;
                struct {
                    PacketUnion packets[3];
                    struct {
                        uint32_t status : 2;
                        uint32_t error_code : 6;
                        uint32_t retry_count : 8;
                        uint32_t reserved : 16;
                    };
                };
            };
        };
        struct {
            uint8_t raw_header[8];
            struct {
                LayerData primary_layer;
                union {
                    struct {
                        ColorData palette[16];
                        struct {
                            uint8_t indices[64];
                            union {
                                struct {
                                    uint32_t format : 4;
                                    uint32_t compression : 4;
                                    uint32_t encrypted : 1;
                                    uint32_t signed_data : 1;
                                    uint32_t version : 6;
                                    uint32_t reserved_bits : 16;
                                };
                                uint32_t flags_combined;
                            };
                        };
                    };
                    uint8_t blob_data[256];
                };
            } graphics;
        };
    };
    
    // More anonymous madness
    struct {
        union {
            struct {
                MixedData mix1;
                struct {
                    union {
                        ControlBits bits[4];
                        struct {
                            uint64_t mask;
                            uint64_t value;
                        };
                    };
                } control_array;
            };
            struct {
                uint16_t table[32];
                union {
                    ColorData color_map[8];
                    struct {
                        uint32_t lookup[16];
                    };
                };
            };
        };
    };
    
    // Final nested anonymous union with bitfields
    union {
        struct {
            uint32_t crc32;
            uint16_t length;
            uint16_t type : 3;
            uint16_t priority : 2;
            uint16_t encrypted : 1;
            uint16_t compressed : 1;
            uint16_t valid : 1;
            uint16_t reserved : 8;
        } footer;
        uint64_t footer_raw;
    };
};

#endif // EXTREME_TEST_H