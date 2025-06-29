#pragma once
#include <stdint.h>
#include "base_types.h"

#define MAX_PACKET_DATA 1024
#define MAX_CONNECTIONS 64
#define PROTOCOL_VERSION 42

// ネットワークアドレス
typedef struct {
    union {
        struct {
            uint8_t a, b, c, d;
        } octets;
        uint32_t addr;
    } ip;
    uint16_t port;
    uint16_t reserved;
} NetworkAddress;

// パケットヘッダー
typedef struct {
    uint32_t magic;
    uint16_t type;
    uint16_t flags;
    uint32_t sequence;
    uint32_t ack_sequence;
    Timestamp timestamp;
    uint16_t data_length;
    uint16_t checksum;
} PacketHeader;

// 接続情報
typedef struct {
    NetworkAddress local;
    NetworkAddress remote;
    union {
        struct {
            uint32_t connected : 1;
            uint32_t authenticated : 1;
            uint32_t encrypted : 1;
            uint32_t compressed : 1;
            uint32_t reserved : 28;
        } flags;
        uint32_t raw_flags;
    } state;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    Timestamp last_activity;
} ConnectionInfo;