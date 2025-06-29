#include <stdint.h>

struct Header {
    uint32_t magic;
    uint16_t version;
    uint16_t flags : 4;
    uint16_t type : 4;
    uint16_t reserved : 8;
};

struct Data {
    Header header;
    char name[16];
    uint32_t values[4];
    union {
        uint64_t timestamp;
        struct {
            uint32_t date;
            uint32_t time;
        };
    };
};